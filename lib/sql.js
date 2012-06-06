//---------------------------------------------------------------------------------------------------------------------------------
// File: sql.js
// Contents: javascript interface to Microsoft Driver for Node.js  for SQL Server
// 
// Copyright Microsoft Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// You may obtain a copy of the License at:
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//---------------------------------------------------------------------------------------------------------------------------------

var sql = require('./sqlserver.node');
var events = require('events');
var util = require('util');

function StreamEvents() {
    events.EventEmitter.call(this);
}
util.inherits(StreamEvents, events.EventEmitter);

function nextOp( q ) {

    q.shift();

    if( q.length != 0 ) {

        var op = q[0];
        op.fn.apply( op.fn, op.args );
    }
}


function query_internal(ext, query, params, callback) {

    if (params.length > 0) {
        var combined = [];
        var split = query.split('?');
        for (var idx = 0; idx < split.length - 1; idx++) {
            combined.push(split[idx]);
            var value = params[idx];
            switch (typeof (value)) {
                case 'string': combined.push("'" + value.replace("'", "''") + "'"); break;
                case 'number': combined.push(value.toString()); break;
                default:
                    if (value instanceof Buffer) {
                        combined.push('0x' + value.toString('hex'));
                    }
                    else {
                        throw new Error('Invalid parameter type.  Support string, number, and Buffer');
                    }
            }
        }
        combined.push(split[split.length - 1]);
        query = combined.join("");
    }

    function onQuery(completed, err, results) {

        if (!completed) {
            ext.query(query, onQuery);
            return;
        }

        if( callback ) {
            callback(err, results);
        }
    }

    return ext.query(query, onQuery);
}


function getChunkyArgs(paramsOrCallback, callback) {
    if (callback) {
        return { params: paramsOrCallback || [], callback: callback };
    }
    else if (typeof (paramsOrCallback) === 'function') {
        return { params: [], callback: paramsOrCallback };
    }
    else {
        return { params: paramsOrCallback || [] };
    }
}

function objectify(results) {
    var names = {};
    var name, idx;
    for (idx in results.meta) {
        var meta = results.meta[idx];
        name = meta.name;
        if (name !== '' && names[name] === undefined) {
            names[name] = idx;
        }
        else {
            var extra = 0;
            var candidate = 'Column' + idx;
            while (names[candidate] !== undefined) {
                candidate = 'Column' + idx + '_' + extra++;
            }
            names[candidate] = idx;
        }
    }

    var rows = [];
    for (idx in results.rows) {
        var row = results.rows[idx];
        var value = {};
        for (name in names) {
            value[name] = row[names[name]];
        }
        rows.push(value);
    }

    return rows;
}

function readall(q, notify, ext, query, params, callback) {

    var meta;
    var column;
    var rows = [];
    var rowindex = 0;

    function onReadColumnMore(completed, err, results) {

        if (!completed) {
            ext.readColumn(column, onReadColumnMore);
            return;
        }

        if (err) {
            notify.emit('error', err);
            if (callback) {
                callback(err);
            }
            nextOp( q );
            return;
        }

        var data = results.data;
        var more = results.more;

        notify.emit('column', column, data, more);

        if (callback) {
            rows[rows.length - 1][column] += data;
        }

        if (more) {
            ext.readColumn(column, onReadColumnMore);
            return;
        }

        column++;
        if (column >= meta.length) {
            ext.readRow(onReadRow);
            return;
        }

        ext.readColumn(column, onReadColumn);
    }

    function onReadColumn(completed, err, results) {

        if (!completed) {
            ext.readColumn(column, onReadColumn);
            return;
        }

        if (err) {
            notify.emit('error', err);
            if (callback) {
                callback(err);
            }
            nextOp( q );
            return;
        }

        var data = results.data;
        var more = results.more;

        notify.emit('column', column, data, more);

        if (callback) {
            rows[rows.length - 1][column] = data;
        }

        if (more) {
            ext.readColumn(column, onReadColumnMore);
            return;
        }

        column++;

        if (column >= meta.length) {
            ext.readRow(onReadRow);
            return;
        }

        ext.readColumn(column, onReadColumn);
    }

    function onReadRow(completed, err, moreRows) {

        if (!completed) {
            ext.readRow(onReadRow);
        }
        
        if (err) {
            notify.emit('error', err);
            if (callback) {
                callback(err);
            }
            nextOp( q );
            return;
        }
        else if (moreRows && meta.length > 0) {

            notify.emit('row', rowindex++);

            column = 0;
            if (callback) {
                rows[rows.length] = [];
            }
            ext.readColumn(column, onReadColumn);
        }
        else {
            notify.emit('done');
            if (callback) {
                callback(err, { meta: meta, rows: rows });
            }
            nextOp( q );
        }
    }

    query_internal(ext, query, params, function (err, results) {

        if (err) {
            notify.emit('error', err);
            if (callback) {
                callback(err);
            }
            nextOp( q );
            return;
        }

        meta = results;

        notify.emit('meta', meta);

        if (meta.length > 0) {
            ext.readRow(onReadRow);
        }
        else {
            notify.emit('done');
            if (callback) {
                callback(err, { meta: meta, rows: rows });
            }
            nextOp( q );
        }
    });
}

function open(connectionString, callback) {

    var ext = new sql.Connection();

    var q = [];

    function Connection() {

        this.close = function (callback) { 

            function onClose( completed, err ) {

                if( !completed ) {
                    ext.close( onClose );
                    return;
                }

                if( callback ) {
                    callback( err );
                }
            }

            ext.close( onClose ); 
        }

        this.queryRaw = function (query, paramsOrCallback, callback) {

            var notify = new StreamEvents();

            var chunky = getChunkyArgs(paramsOrCallback, callback);

            var op = { fn: readall, args: [ q, notify, ext, query, chunky.params, chunky.callback ] }; 
            q.push( op );
            
            if( q.length == 1 ) {

                readall( q, notify, ext, query, chunky.params, chunky.callback );
            }

            return notify;
        }

        this.query = function (query, paramsOrCallback, callback) {

            var chunky = getChunkyArgs(paramsOrCallback, callback);

            function onQueryRaw(completed, err, results) {

                if (!completed) {
                    ext.queryRaw(query, chunky.params, onQueryRaw);
                    return;
                }
                if (chunky.callback) {
                    if (err) chunky.callback(err);
                    else chunky.callback(err, objectify(results));
                }
            }

            return this.queryRaw(query, chunky.params, onQueryRaw);
        }

        this.beginTransaction = function(callback) {

            function onBeginTxn( completed, err ) {

                if( !completed ) {
                    ext.beginTransaction( onBeginTxn );
                    return;
                }

                if( callback ) {
                    callback( err );
                }

                nextOp( q );
            }

            var op = { fn: function( callback ) { ext.beginTransaction( callback ) }, args: [ onBeginTxn ] }; 
            q.push( op );

            if( q.length == 1 ) {

                ext.beginTransaction( onBeginTxn );
            }
        }

        this.commit = function(callback) {

            function onCommit( completed, err ) {

                if( !completed ) {
                    ext.commit( onCommit );
                    return;
                }

                if( callback ) {
                    callback( err );
                }

                nextOp( q );
            }

            var op = { fn: function( callback ) { ext.commit( callback ); }, args: [ onCommit ] }; 
            q.push( op );

            if( q.length == 1 ) {

                ext.commit( onCommit );
            }
        }

        this.rollback = function(callback) {

            function onRollback( completed, err ) {

                if( !completed ) {
                    ext.rollback( onRollback );
                    return;
                }

                if( callback ) {
                    callback( err );
                }

                nextOp( q );
            }

            var op = { fn: function( callback ) { ext.rollback( callback ); }, args: [ onRollback ] }; 
            q.push( op );

            if( q.length == 1 ) {

                ext.rollback( onRollback );
            }
        }
    }

    function onOpen(completed, err) {

        if (!completed) {
            ext.open(connectionString, onOpen);
            return;
        }

        if( callback ) {
            callback(err, connection);
        }
    }

    var connection = new Connection();
    ext.open(connectionString, onOpen);
    return connection;
}

function query(connectionString, query, paramsOrCallback, callback) {

    var chunky = getChunkyArgs(paramsOrCallback, callback);

    return queryRaw(connectionString, query, chunky.params, function (err, results) {
        if (chunky.callback) {
            if (err) chunky.callback(err);
            else chunky.callback(err, objectify(results));
        }
    });
}

function queryRaw(connectionString, query, paramsOrCallback, callback) {

    var ext = new sql.Connection();
    var notify = new StreamEvents();
    var q = [];

    var chunky = getChunkyArgs(paramsOrCallback, callback);

    function onOpen(completed, err, connection) {

        if (!completed) {
            ext.open(connectionString, onOpen);
            return;
        }
        if (err) {
            notify.on('error', err);
            if (chunky.callback) {
                chunky.callback(err);
            }
            return;
        }
        else {
            readall(q, notify, ext, query, chunky.params, function (err, results) {
                if (chunky.callback) {
                    chunky.callback(err, results);
                }
                ext.close();
            });
        }
    }

    ext.open(connectionString, onOpen);

    return notify;
}

exports.open = open;
exports.query = query; 
exports.queryRaw = queryRaw;
