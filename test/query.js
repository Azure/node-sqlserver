//---------------------------------------------------------------------------------------------------------------------------------
// File: queries.js
// Contents: test suite for queries
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

suite('query', function () {

    var conn_str = "Driver={SQL Server Native Client 11.0};Server=" + config.server + ";Trusted_Connection={Yes};";

    test('simple query', function (done) {

        sql.query(conn_str, "SELECT 1 as X, 'ABC', 0x0123456789abcdef ", function (err, results) {

            assert.ifError(err);

            var buffer = new Buffer('0123456789abcdef', 'hex');
            var expected = [{ 'X': 1, 'Column1': 'ABC', 'Column2': buffer}];

            assert.deepEqual(results, expected, "Results don't match");

            done();
        });
    });

    test('simple raw query', function (done) {

        sql.queryRaw(conn_str, "SELECT 1 as X, 'ABC', 0x0123456789abcdef ", function (err, results) {

            assert.ifError(err);

            var buffer = new Buffer('0123456789abcdef', 'hex');

            var expected = { meta:
                             [{ name: 'X', size: 10, nullable: false, type: 'number' },
                               { name: '', size: 3, nullable: false, type: 'text' },
                               { name: '', size: 8, nullable: false, type: 'binary'}],
                rows: [[1, 'ABC', buffer]]
            }

            assert.deepEqual(results, expected, "raw results didn't match");

            done();
        });

    });

    test('simple query of types like var%', function (done) {

        var like = 'var%';

        sql.query(conn_str, "SELECT name FROM sys.types WHERE name LIKE ?", [like], function (err, results) {

            assert.ifError(err);

            for (var row = 0; row < results.length; ++row) {

                assert(results[row].name.substr(0, 3) == 'var');
            }

            done();
        });

    });

    test('streaming test', function (done) {

        var like = 'var%';
        var current_row = 0;
        var meta_expected = [{ name: 'name', size: 128, nullable: false, type: 'text'}];

        var stmt = sql.query(conn_str, 'select name FROM sys.types WHERE name LIKE ?', [like]);

        stmt.on('meta', function (meta) { assert.deepEqual(meta, meta_expected); });
        stmt.on('row', function (idx) { assert(idx == current_row); ++current_row; });
        stmt.on('column', function (idx, data, more) { assert(data.substr(0,3) == 'var'); });
        stmt.on('done', function () { done(); });
        stmt.on('error', function (err) { assert.ifError(err); });
    });

    test('serialized queries', function (done) {

        var expected = [{ meta: [{ name: '', size: 10, nullable: false, type: 'number'}],
            rows: [[1]]
        },
                         { meta: [{ name: '', size: 10, nullable: false, type: 'number'}],
                             rows: [[2]]
                         },
                         { meta: [{ name: '', size: 10, nullable: false, type: 'number'}],
                             rows: [[3]]
                         },
                         { meta: [{ name: '', size: 10, nullable: false, type: 'number'}],
                             rows: [[4]]
                         },
                         { meta: [{ name: '', size: 10, nullable: false, type: 'number'}],
                             rows: [[5]]
                         }];

        var results = [];

        var c = sql.open(conn_str, function (e) {

            assert.ifError(e);

            c.queryRaw("SELECT 1", function (e, r) {

                assert.ifError(e);

                results.push(r);
            });

            c.queryRaw("SELECT 2", function (e, r) {

                assert.ifError(e);

                results.push(r);
            });

            c.queryRaw("SELECT 3", function (e, r) {

                assert.ifError(e);

                results.push(r);
            });

            c.queryRaw("SELECT 4", function (e, r) {

                assert.ifError(e);

                results.push(r);
            });

            c.queryRaw("SELECT 5", function (e, r) {

                assert.ifError(e);

                results.push(r);

                assert.deepEqual(expected, results);
                done();
            });
        });
    });

    test( 'query with errors', function( done ) {

        var c = sql.open( conn_str, function( e ) {

            assert.ifError( e );

            async.series( [

                function( async_done ) {

                    assert.doesNotThrow( function() {

                        c.queryRaw( "I'm with NOBODY", function( e, r ) {

                            assert.equal( e.toString(),
                                          "Error: 42000: [Microsoft][SQL Server Native Client 11.0][SQL Server]Unclosed quotation mark after the character string 'm with NOBODY'." );
                            async_done();
                        });
                    });
                },

                function( async_done ) {

                    assert.doesNotThrow( function() {

                        var s = c.queryRaw( "I'm with NOBODY" );
                        s.on( 'error', function( e ) {

                            assert.equal( e.toString(), "Error: 42000: [Microsoft][SQL Server Native Client 11.0][SQL Server]Unclosed quotation mark after the character string 'm with NOBODY'." );
                            async_done();
                            done();
                        });
                    });
                }
            ]);
        });
    });

    test( 'query with errors', function( done ) {

        var c = sql.open( conn_str, function( e ) {

            assert.ifError( e );

            async.series( [

                function( async_done ) {

                    assert.doesNotThrow( function() {

                        c.queryRaw( "I'm with NOBODY", function( e, r ) {

                            assert.equal( e.toString(),
                                          "Error: 42000: [Microsoft][SQL Server Native Client 11.0][SQL Server]Unclosed quotation mark after the character string 'm with NOBODY'." );
                            async_done();
                        });
                    });
                },

                function( async_done ) {

                    assert.doesNotThrow( function() {

                        var s = c.queryRaw( "I'm with NOBODY" );
                        s.on( 'error', function( e ) {

                            assert.equal( e.toString(), "Error: 42000: [Microsoft][SQL Server Native Client 11.0][SQL Server]Unclosed quotation mark after the character string 'm with NOBODY'." );
                            async_done();
                            done();
                        });
                    });
                }
            ]);
        });
    });

    test( 'multiple results from query in callback', function( done ) {

        var moreShouldBe = true;
        var called = 0;

        sql.queryRaw(conn_str, "SELECT 1 as X, 'ABC', 0x0123456789abcdef; SELECT 2 AS Y, 'DEF', 0xfedcba9876543210", 
            function( err, results, more ) {

                assert.ifError( err );

                assert.equal( more, moreShouldBe );

                ++called;

                if( more ) {

                    var buffer = new Buffer('0123456789abcdef', 'hex');
                    var expected = { meta:
                                     [ { name: 'X', size: 10, nullable: false, type: 'number' },
                                       { name: '', size: 3, nullable: false, type: 'text' },
                                       { name: '', size: 8, nullable: false, type: 'binary' } ],
                                     rows: [ [ 1, 'ABC', buffer ] ] };

                    assert.deepEqual( results, expected, "Result 1 does not match expected" );

                    assert( called == 1 );
                    moreShouldBe = false;
                }
                else {

                    var buffer = new Buffer('fedcba9876543210', 'hex');
                    var expected = { meta:
                                     [ { name: 'Y', size: 10, nullable: false, type: 'number' },
                                       { name: '', size: 3, nullable: false, type: 'text' },
                                       { name: '', size: 8, nullable: false, type: 'binary' } ],
                                     rows: [ [ 2, 'DEF', buffer ] ] };

                    assert.deepEqual( results, expected, "Result 2 does not match expected" );

                    assert( called == 2 );
                    done();
                }
            });
    });

    test( 'multiple results from query in events', function( done ) {

        var r = sql.queryRaw( conn_str, "SELECT 1 as X, 'ABC', 0x0123456789abcdef; SELECT 2 AS Y, 'DEF', 0xfedcba9876543210" );

        var expected = [ [ { name: 'X', size: 10, nullable: false, type: 'number' },
                           { name: '', size: 3, nullable: false, type: 'text' },
                           { name: '', size: 8, nullable: false, type: 'binary' } ],
                         { row: 0 },
                         { column: 0, data: 1, more: false },
                         { column: 1, data: 'ABC', more: false },
                         { column: 2,
                           data: new Buffer('0123456789abcdef', 'hex'),
                           more: false },
                         [ { name: 'Y', size: 10, nullable: false, type: 'number' },
                           { name: '', size: 3, nullable: false, type: 'text' },
                           { name: '', size: 8, nullable: false, type: 'binary' } ],
                         { row: 1 },
                         { column: 0, data: 2, more: false },
                         { column: 1, data: 'DEF', more: false },
                         { column: 2,
                           data: new Buffer('fedcba9876543210', 'hex'),
                           more: false } ];
        var received = [];

        r.on('meta', function( m ) { received.push( m ); } );
        r.on('row', function( idx ) {  received.push( { row: idx } ); } );
        r.on('column', function( idx, data, more ) { received.push( { column: idx, data: data, more: more } ); } );
        r.on('done', function() { assert.deepEqual( received, expected ); done() } );
        r.on('error', function( e ) { assert.ifError( e ); } );
    });

    test( 'boolean return value from query', function( done ) {

        var r = sql.queryRaw( conn_str, "SELECT CONVERT(bit, 1) AS bit_true, CONVERT(bit, 0) AS bit_false", 
                              function ( err, results ) {

                                  assert.ifError( err );

                                  var expected = { meta: [ { name: 'bit_true', size: 1, nullable: true, type: 'boolean' },
                                                   { name: 'bit_false', size: 1, nullable: true, type: 'boolean' } ],
                                                   rows: [ [ true, false ] ] };
                                  assert.deepEqual( results, expected, "Results didn't match" );
                                  done();
                              });

    });

    test( 'verify empty results retrieved properly', function( test_done ) {

        sql.open( conn_str, function( err, conn ) {

          async.series([
            function( async_done ) {
              conn.queryRaw( "drop table test_sql_no_data", function( err ) {
                async_done();
              });
            },
            function( async_done ) {
              conn.queryRaw( "create table test_sql_no_data (id int identity, name varchar(20))", function( err ) {
                assert.ifError( err );
                async_done();
              });
            },
            function( async_done ) {
              conn.queryRaw( "delete from test_sql_no_data where 1=0", function( err, results ) {

                assert.ifError( err );
                var expectedResults = { meta: null, rowcount: 0 }
                assert.deepEqual( results, expectedResults );
                async_done();
                test_done();
              });
            }
            ]);
        });
    });

});
