//---------------------------------------------------------------------------------------------------------------------------------
// File: txn.js
// Contents: test suite for transactions
// 
// Copyright Microsoft Corporation and contributors
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

var sql = require('../');
var assert = require( 'assert' );
var async = require( 'async' );
var config = require( './test-config' );

var conn_str = "Driver={SQL Server Native Client 11.0};Server=" + config.server + ";Trusted_Connection={Yes}";

// single setup necessary for the test
async.series( [

    function( async_done ) { 
        try {
            var q = sql.query( conn_str, "drop table test_txn", function( err, results ) {

                async_done();
            });
            // if this isn't defined, then an "Uncaught error" exception is thrown by the event.
            // Maybe that should be optional?  Emitting events before callbacks is okay? 
            q.on('error', function( e ) {} );// ignore any errors from dropping the table
        }
        // skip any errors because the table might not exist
        catch( e ) {
            async_done();
        }
    },
    function( async_done ) { 
        sql.query( conn_str, "create table test_txn (id int identity, name varchar(100))", function( err, results ) {

            assert.ifError( err );
            async_done();
        });
    }
]);

suite( 'txn', function() {

    var conn;

    setup( function(test_done) {

        sql.open( conn_str, function( err, new_conn ) {
            
            assert.ifError( err );
            
            conn = new_conn;

            test_done();
        });
    });

    teardown( function(done) {

        conn.close( function( err ) { assert.ifError( err ); done(); });
    });

    test('begin a transaction and rollback with no query', function( done ) {

        conn.beginTransaction( function( err ) { assert.ifError( err ); });

        conn.rollback( function( err ) { assert.ifError( err ); done(); });
    });

    test('begin a transaction and rollback with no query and no callback', function( done ) {

        try {

            conn.beginTransaction();

            conn.rollback( function( err ) {
                assert.ifError( err );
                done();
            });
        }
        catch( e ) {

            assert.ifError( e );
        }
    });


    test('begin a transaction and commit', function( test_done ) {

        conn.beginTransaction( function( err ) { 

            assert.ifError( err );

            async.series( [

                function( done ) { 
                    conn.queryRaw( "INSERT INTO test_txn (name) VALUES ('Anne')", function( err, results ) { 
                        assert.ifError( err ); 
                        assert.deepEqual( results, { meta: null, rowcount: 1 }, "Insert results don't match" );
                        done();
                    });
                },
                function( done ) {
                    conn.queryRaw( "INSERT INTO test_txn (name) VALUES ('Bob')", function( err, results ) { 
                        assert.ifError( err );
                        assert.deepEqual( results, { meta: null, rowcount: 1 }, "Insert results don't match" );
                        done();
                    });
                },
                function( done ) {
                    conn.commit( function( err ) { 
                        assert.ifError( err );
                        done();
                    });
                },
                function( done ) {
                    conn.queryRaw( "select * from test_txn", function( err, results ) {
                        assert.ifError( err );

                        // verify results
                        var expected = { 'meta':
                                         [ { 'name': 'id', 'size': 10, 'nullable': false, 'type': 'number' },
                                           { 'name': 'name', 'size': 100, 'nullable': true, 'type': 'text' } ],
                                         'rows': [ [ 1, 'Anne' ], [ 2, 'Bob' ] ] };

                        assert.deepEqual( results, expected, "Transaction not committed properly" );

                        done();
                        test_done();
                    });
                }
            ]);
        });
    });

    test('begin a transaction and rollback', function( test_done ) {

        conn.beginTransaction( function( err ) { 

            assert.ifError( err );

            async.series( [

                function( done ) { 
                    conn.queryRaw( "INSERT INTO test_txn (name) VALUES ('Carl')", function( err, results ) { 
                        assert.ifError( err ); 
                        assert.deepEqual( results, { meta: null, rowcount: 1 }, "Insert results don't match" );
                        done();
                    });
                },
                function( done ) {
                    conn.queryRaw( "INSERT INTO test_txn (name) VALUES ('Dana')", function( err, results ) { 
                        assert.ifError( err );
                        assert.deepEqual( results, { meta: null, rowcount: 1 }, "Insert results don't match" );
                        done();
                    });
                },
                function( done ) {
                    conn.rollback( function( err ) { 
                        assert.ifError( err );
                        done();
                    });
                },
                function( done ) {
                    conn.queryRaw( "select * from test_txn", function( err, results ) {
                        assert.ifError( err );

                        // verify results
                        var expected = { 'meta':
                                         [ { 'name': 'id', 'size': 10, 'nullable': false, 'type': 'number' },
                                           { 'name': 'name', 'size': 100, 'nullable': true, 'type': 'text' } ],
                                         'rows': [ [ 1, 'Anne' ], [ 2, 'Bob' ] ] };

                        assert.deepEqual( results, expected, "Transaction not rolled back properly" );

                        done();
                        test_done();
                    });
                }
            ]);
        });
    });

    test('begin a transaction and then query with an error', function( test_done ) {

        conn.beginTransaction( function( err ) { 

            assert.ifError( err );

            async.series( [

                function( done ) { 
                    var q = conn.queryRaw( "INSERT INTO test_txn (naem) VALUES ('Carl')" );
                    // events are emitted before callbacks are called currently
                    q.on('error', function( err ) {
                        
                        var expected = "Error: 42S22: [Microsoft][SQL Server Native Client 11.0][SQL Server]Invalid column name 'naem'.";
                        assert.equal( err.toString(), expected, "Transaction should have caused an error" );

                        conn.rollback( function( err ) { 
                            assert.ifError( err );
                            done();
                        });
                    });
                },
                function( done ) {
                    conn.queryRaw( "select * from test_txn", function( err, results ) {
                        assert.ifError( err );

                        // verify results
                        var expected = { 'meta':
                                         [ { 'name': 'id', 'size': 10, 'nullable': false, 'type': 'number' },
                                           { 'name': 'name', 'size': 100, 'nullable': true, 'type': 'text' } ],
                                         'rows': [ [ 1, 'Anne' ], [ 2, 'Bob' ] ] };

                        assert.deepEqual( results, expected, "Transaction not rolled back properly" );

                        done();
                        test_done();
                    });
                }
            ]);
        });
    });

    test('begin a transaction and commit (with no async support)', function( test_done ) {

        conn.beginTransaction( function( err ) { 

            assert.ifError( err );
        });

        conn.queryRaw( "INSERT INTO test_txn (name) VALUES ('Anne')", function( err, results ) { 
            assert.ifError( err ); 
        });

        conn.queryRaw( "INSERT INTO test_txn (name) VALUES ('Bob')", function( err, results ) { 
            assert.ifError( err );
        });

        conn.commit( function( err ) { 
            assert.ifError( err );
        });
            
        conn.queryRaw( "select * from test_txn", function( err, results ) {

            assert.ifError( err );

            // verify results
            var expected = { 'meta':
                             [ { 'name': 'id', 'size': 10, 'nullable': false, 'type': 'number' },
                               { 'name': 'name', 'size': 100, 'nullable': true, 'type': 'text' } ],
                             'rows': [ [ 1, 'Anne' ], [ 2, 'Bob' ], [ 5, 'Anne' ], [ 6, 'Bob' ] ] };

            assert.deepEqual( results, expected, "Transaction not committed properly" );

            test_done();
        });

     });
});

