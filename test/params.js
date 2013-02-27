//---------------------------------------------------------------------------------------------------------------------------------
// File: params.js
// Contents: test suite for parameters
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

var sql=require('../');
var assert=require('assert');
var async=require('async');
var config = require('./test-config');

var conn_str = config.conn_str;

function testBoilerPlate( tableName, tableFields, insertFunction, verifyFunction, doneFunction ) {

    var tableFieldsSql = " (id int identity, ";

    for( var field in tableFields ) {

        tableFieldsSql += field + ' ' + tableFields[field] + ',';
    }
    tableFieldsSql = tableFieldsSql.substr( 0, tableFieldsSql.length - 1 );
    tableFieldsSql += ')';

    async.series( [

        function( async_done ) {

            var dropQuery = "DROP TABLE " + tableName;
            sql.query( conn_str, dropQuery, function( e, r ) { async_done(); } );
        },
        function( async_done ) {

            var createQuery = "CREATE TABLE " + tableName + tableFieldsSql;
            sql.query( conn_str, createQuery, 
                       function( e, r ) { 
                           assert.ifError( e, "Error creating table" );
                           async_done();
                       });
        },
        function( async_done ) {

            var clusteredIndexSql = ["CREATE CLUSTERED INDEX IX_", tableName, " ON ", tableName, " (id)"].join("");
            sql.query( conn_str, clusteredIndexSql, 
                       function( e, r ) { 
                           assert.ifError( e, "Error creating index" );
                           async_done();
                       });
        },
        function( async_done ) {

            insertFunction( async_done );
        },
        function( async_done ) {

            verifyFunction( function() { async_done(); doneFunction(); } );
        }
    ]);
}

suite( 'params', function() {

    var c;

    var localDate = new Date();
    var utcDate = new Date( Date.UTC( localDate.getUTCFullYear(), 
                                      localDate.getUTCMonth(),
                                      localDate.getUTCDate(),
                                      localDate.getUTCHours(),
                                      localDate.getUTCMinutes(),
                                      localDate.getUTCSeconds(),
                                      localDate.getUTCMilliseconds() ));

    setup( function( test_done ) {

        sql.open( conn_str, function( err, new_conn ) {

            assert.ifError( err );

            c = new_conn;

            test_done();
        });
    });

    teardown( function( done ) {

        c.close( function( err ) { assert.ifError( err ); done(); });
    });

    test('insert null as parameter', function( test_done ) {

        testBoilerPlate( "null_param_test", { "null_test" : "varchar(1)" },
            function( done ) {

                c.queryRaw( "INSERT INTO null_param_test (null_test) VALUES (?)", [null], function( e, r ) {
                    assert.ifError( e );
                    done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT null_test FROM null_param_test", function( e, r ) {

                    assert.ifError( e );
                    var expected = { meta: [ { name: 'null_test', size: 1, nullable: true, type: 'text', sqlType: 'varchar' } ],
                                     rows: [[null]] };
                    assert.deepEqual( expected, r );
                    done();
                });
            },
            test_done);
    });

    test('insert string as parameter', function( test_done ) {

        testBoilerPlate( "string_param_test", { "string_test" : "nvarchar(100)" },
            function( done ) {

                c.queryRaw( "INSERT INTO string_param_test (string_test) VALUES (?)", ['This is a test'], function( e, r ) {
                    assert.ifError( e );
                    done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT string_test FROM string_param_test", function( e, r ) {

                    assert.ifError( e );
                    var expected = { meta: [ { name: 'string_test', size: 100, nullable: true, type: 'text', sqlType: 'nvarchar' } ],
                                     rows: [ [ 'This is a test' ] ] };
                    assert.deepEqual( expected, r );
                    done();
                });
            },
            test_done);
    });

    test( 'insert a bool as a parameter', function( test_done ) {

        testBoilerPlate( "bool_param_test", { "bool_test" : "bit" },
            function( done ) {
                c.queryRaw( "INSERT INTO bool_param_test (bool_test) VALUES (?)", [true], function( e, r ) {
                    assert.ifError( e );
                    done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT bool_test FROM bool_param_test", function( e, r ) {

                    assert.ifError( e );
                    var expected = { meta: [ { name: 'bool_test', size: 1, nullable: true, type: 'boolean', sqlType: 'bit' } ],
                                     rows: [ [ true ] ] };
                    assert.deepEqual( expected, r );
                    done();
                });
            },
            test_done);            
    });

    test( 'insert largest positive int as parameter', function( test_done ) {

        testBoilerPlate( "int_param_test", { "int_test" : "int" },
            function( done ) {
                c.queryRaw( "INSERT INTO int_param_test (int_test) VALUES (?)", [0x7fffffff], function( e, r ) {
                    assert.ifError( e );
                    done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT int_test FROM int_param_test", function( e, r ) {

                    assert.ifError( e );
                    var expected = { meta: [ { name: 'int_test', size: 10, nullable: true, type: 'number', sqlType: 'int' } ],
                                    rows: [ [ 2147483647 ] ] };
                    assert.deepEqual( expected, r );
                    done();
                });
            },
            test_done);            
    });

    test( 'insert largest negative int as parameter', function( test_done ) {

        testBoilerPlate( "int_param_test", { "int_test" : "int" },
            function( done ) {
                c.queryRaw( "INSERT INTO int_param_test (int_test) VALUES (?)", [-0x80000000], function( e, r ) {
                    assert.ifError( e );
                    done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT int_test FROM int_param_test", function( e, r ) {

                    assert.ifError( e );
                    var expected = { meta: [ { name: 'int_test', size: 10, nullable: true, type: 'number', sqlType: 'int' } ],
                                     rows: [ [ -2147483648 ] ] };
                    assert.deepEqual( expected, r );
                    done();
                });
            },  
            test_done);            
    });

    test( 'insert bigint as parameter', function( test_done ) {

        testBoilerPlate( "bigint_param_test", { "bigint_test" : "bigint" },
            function( done ) {
                c.queryRaw( "INSERT INTO bigint_param_test (bigint_test) VALUES (?)", [0x80000000], function( e, r ) {
                    assert.ifError( e );
                    done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT bigint_test FROM bigint_param_test", function( e, r ) {

                    assert.ifError( e );
                    var expected = { meta: [ { name: 'bigint_test', size: 19, nullable: true, type: 'number', sqlType: 'bigint' } ],
                                     rows: [ [ 0x80000000 ] ] };
                    assert.deepEqual( expected, r );
                    done();
                });
            },  
            test_done);                        
    });

    test( 'insert largest bigint as parameter', function( test_done ) {

        testBoilerPlate( "bigint_param_test", { "bigint_test" : "bigint" },
            function( done ) {
                c.queryRaw( "INSERT INTO bigint_param_test (bigint_test) VALUES (?)", [0x4fffffffffffffff], function( e, r ) {
                    assert.ifError( e );
                    done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT bigint_test FROM bigint_param_test", function( e, r ) {

                    assert.ifError( e );
                    var expected = { meta: [ { name: 'bigint_test', size: 19, nullable: true, type: 'number', sqlType: 'bigint' } ],
                                     rows: [ [ 0x4fffffffffffffff ] ] };
                    assert.deepEqual( expected, r );
                    done();
                });
            },  
            test_done);                        
    });

    test( 'insert decimal as parameter', function( test_done ) {

        testBoilerPlate( "decimal_param_test", { "decimal_test" : "decimal(18,7)" },
            function( done ) {
                c.queryRaw( "INSERT INTO decimal_param_test (decimal_test) VALUES (?)", [3.141593],
                    function( e, r ) {
                        assert.ifError( e );
                        done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT decimal_test FROM decimal_param_test", function( e, r ) {

                    assert.ifError( e );
                    var expected = { meta: [ { name: 'decimal_test', size: 18, nullable: true, type: 'number', sqlType: 'decimal' } ],
                                     rows: [ [ 3.141593 ] ] };
                    assert.deepEqual( expected, r );
                    done();
                });
            },  
            test_done);                        
    });

    test( 'insert decimal as bigint parameter', function( test_done ) {

        testBoilerPlate( "decimal_as_bigint_param_test", { "decimal_bigint" : "bigint" },
            function( done ) {
                c.queryRaw( "INSERT INTO decimal_as_bigint_param_test (decimal_bigint) VALUES (?)", [123456789.0],
                    function( e, r ) {
                        assert.ifError( e );
                        done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT decimal_bigint FROM decimal_as_bigint_param_test", function( e, r ) {

                    assert.ifError( e );
                    var expected = { meta:
                                       [ { name: 'decimal_bigint',
                                           size: 19,
                                           nullable: true,
                                           type: 'number', sqlType: 'bigint' } ],
                                      rows: [ [ 123456789 ] ] };
                    assert.deepEqual( expected, r );
                    done();
                });
            },  
            test_done);                        
    });

    test( 'insert date as parameter', function( test_done ) {

        testBoilerPlate( "date_param_test", { "date_test" : "datetimeoffset" },
            function( done ) {
                c.queryRaw( "INSERT INTO date_param_test (date_test) VALUES (?)", [utcDate],
                    function( e, r ) {
                        assert.ifError( e );
                        done();
                });
            },
            function( done ) {

                c.queryRaw( "SELECT date_test FROM date_param_test", function( e, r ) {

                    assert.ifError( e );

                    assert.equal( utcDate.toISOString(), r.rows[0][0].toISOString(), "dates are not equal" );
                    assert.equal( r.rows[0][0].nanosecondsDelta, 0, "nanoseconds not 0" );
                    done();
                });
            },  
            test_done);                        
    });

    test( 'verify js date inserted into datetime field', function( test_done ) {

        var localDate = new Date();
        var utcDate = new Date( Date.UTC( localDate.getUTCFullYear(), 
                                          localDate.getUTCMonth(),
                                          localDate.getUTCDate(),
                                          localDate.getUTCHours(),
                                          localDate.getUTCMinutes(),
                                          localDate.getUTCSeconds(),
                                          localDate.getUTCMilliseconds() ));

        testBoilerPlate( "datetime_test", { "datetime_test" : "datetime" },
            function( done ) {

                c.queryRaw( "INSERT INTO datetime_test (datetime_test) VALUES (?)", [ utcDate ], function( e, r ) {

                    assert.ifError( e );
                    assert( r.rowcount == 1 );

                    done();
                }); 
            },
            function( done ) {


                c.queryRaw( "SELECT * FROM datetime_test", function( e, r ) {

                    assert.ifError( e );
                    assert( r.rows[0][0], utcDate );

                    done();
                }); 
            },
            test_done );

    });

    test( 'verify empty string inserted into nvarchar field', function( test_done ) {

        testBoilerPlate( "emptystring_test", { "emptystring_test" : "nvarchar(1)" },
            function( done ) {

                c.queryRaw( "INSERT INTO emptystring_test (emptystring_test) VALUES (?)", [ '' ], function( e, r ) {

                    assert.ifError( e );
                    assert( r.rowcount == 1 );

                    done();
                }); 
            },
            function( done ) {

                c.queryRaw( "SELECT * FROM emptystring_test", function( e, r ) {

                    assert.ifError( e );
                    assert( r.rows[0][0], '' );

                    done();
                });
            },
            test_done );
    });

    test( 'insert min and max number values', function( test_done ) {

        testBoilerPlate( "minmax_test", { "f" : "float" },
            function( done ) {
                async.series([

                    function( async_done ) {

                        c.queryRaw( "INSERT INTO minmax_test (f) VALUES (?)", [ Number.MAX_VALUE ], 
                            function( e, r ) {

                                assert.ifError( e );

                                async_done();
                        });
                    },
                    function( async_done ) {

                        c.queryRaw( "INSERT INTO minmax_test (f) VALUES (?)", [ -Number.MAX_VALUE ], 
                            function( e, r ) {

                                assert.ifError( e );

                                async_done();
                                done();
                        });
                    },
                ]);
            },
            function( done ) {

                c.queryRaw( "SELECT f FROM minmax_test", function( e, r ) {

                    assert.ifError( e );

                    var expected =  { meta: [ { name: 'f', size: 53, nullable: true, type: 'number', sqlType: 'float' } ],
                                      rows: [ [ 1.7976931348623157e+308 ], [ -1.7976931348623157e+308 ] ] };

                    assert.deepEqual( r, expected, "minmax results don't match" );

                    done();
                });
            },            
            test_done );
    });

    test( 'invalid numbers cause errors', function( test_done ) {

        testBoilerPlate( "invalid_numbers_test", { "f" : "float" },

            function( done ) {
                async.series([
                    function( async_done ) {

                        c.queryRaw( "INSERT INTO invalid_numbers_test (f) VALUES (?)", [ Number.POSITIVE_INFINITY ], function( e, r ) {

                            assert( e == "Error: IMNOD: [msnodesql] Parameter 1: Invalid number parameter" );

                            async_done();
                        });
                    },

                    function( async_done ) {

                        c.queryRaw( "INSERT INTO invalid_numbers_test (f) VALUES (?)", [ Number.NEGATIVE_INFINITY ], function( e, r ) {

                            assert( e == "Error: IMNOD: [msnodesql] Parameter 1: Invalid number parameter" );

                            async_done();
                            done();
                        });
                    }
                ]);
            },
            function( done ) {
                done();
            },
            test_done );
    });

    test( 'insert large string into max column', function( test_done ) {

        String.prototype.repeat = function( num )
        {
            return new Array( num + 1 ).join( this );
        }

        testBoilerPlate( "test_large_insert", { "large_insert" : "nvarchar(max) " },
            function( done ) {

                var large_text = "A".repeat( 10000 );

                c.query( "INSERT INTO test_large_insert (large_insert) VALUES (?)", [ large_text ], function( e, r ) {

                    assert.ifError( e, "Error inserting large string" );

                    done();
                })
            },
            function( done ) {

                c.query( "SELECT large_insert FROM test_large_insert", function( e, r ) {

                    assert.ifError( e );

                    assert( r[0].large_insert.length == 10000, "Incorrect length for large insert" );

                    done();
                });
            },
            test_done );
    });

    test( 'verify js date inserted into datetime field', function( test_done ) {

        var localDate = new Date();
        var utcDate = new Date( Date.UTC( localDate.getUTCFullYear(), 
                                          localDate.getUTCMonth(),
                                          localDate.getUTCDate(),
                                          localDate.getUTCHours(),
                                          localDate.getUTCMinutes(),
                                          localDate.getUTCSeconds(),
                                          localDate.getUTCMilliseconds() ));

        testBoilerPlate( "datetime_test", { "datetime_test" : "datetime" },

            function( done ) {

                c.queryRaw( "INSERT INTO datetime_test (datetime_test) VALUES (?)", [ utcDate ], function( e, r ) {

                    assert.ifError( e );
                    assert( r.rowcount == 1 );

                    done();
                }); 
            },
            function( done ) {

                c.queryRaw( "SELECT * FROM datetime_test", function( e, r ) {

                    assert.ifError( e );
                    assert( r.rows[0][0], utcDate );

                    done();
                }); 
            },
            test_done );
    });

    test( 'verify js date before 1970 inserted into datetime field', function( test_done ) {

        var ancientDate = new Date( 1492, 10, 11, 6, 32, 46, 578 );
        var utcDate = new Date( Date.UTC( ancientDate.getUTCFullYear(), 
                                          ancientDate.getUTCMonth(),
                                          ancientDate.getUTCDate(),
                                          ancientDate.getUTCHours(),
                                          ancientDate.getUTCMinutes(),
                                          ancientDate.getUTCSeconds(),
                                          ancientDate.getUTCMilliseconds() ));

        testBoilerPlate( 'datetime_test', { 'datetime_test' : 'datetimeoffset(3)' },

            function( done ) {

                c.queryRaw( "INSERT INTO datetime_test (datetime_test) VALUES (?)", [ utcDate ], function( e, r ) {

                    assert.ifError( e );
                    assert( r.rowcount == 1 );

                    done();
                }); 
            },
            function( done ) {

                c.queryRaw( "SELECT datetime_test FROM datetime_test", function( e, r ) {

                    assert.ifError( e );
                    assert.equal( r.rows[0][0].valueOf(), utcDate.valueOf() );

                    done();
                }); 
            },
            test_done );
    });

  // verify fix for a bug that would return the wrong day when a datetimeoffset was inserted where the date
  // was before 1/1/1970 and the time was midnight.
  test( 'verify dates with midnight time', function( test_done ) {
    sql.open( conn_str, function( err, conn ) {

        assert.ifError( err );

        var midnightDate = new Date( Date.parse( "2030-08-13T00:00:00.000Z" ));

        testBoilerPlate( 'midnight_date_test', { 'midnight_date_test': 'datetimeoffset(3)' },

            function (done) {

                var insertQuery = "INSERT INTO midnight_date_test (midnight_date_test) VALUES (?);";
                conn.queryRaw( insertQuery, [ midnightDate ], function( e ) {

                    assert.ifError( e );
                    done();
                });
            },
            // test valid dates
            function( done ) {

                conn.queryRaw( "SELECT midnight_date_test FROM midnight_date_test", function( e, r ) {

                    assert.ifError( e );

                    var expectedDates = [];
                    var expectedDate = midnightDate
                    expectedDates.push( [ expectedDate ]);
                    var expectedResults = { meta: [ { name: 'midnight_date_test', size: 30, nullable: true, type: 'date', sqlType: 'datetimeoffset' } ],
                        rows: expectedDates 
                    };
                    assert.deepEqual( expectedResults.meta, r.meta );
                    assert( r.rows.length == 1 );
                    for( var row in r.rows ) {
                        for( var d in row ) {
                            assert.deepEqual( expectedResults.rows[row][d], r.rows[row][d] );
                        }
                    }
                    done();
                })
            },
            test_done );
      });
  });

  test( 'verify bug fix for last day of the year error', function( test_done ) {
    sql.open( conn_str, function( err, conn ) {

        assert.ifError( err );

        var eoyDate = new Date( Date.parse( "1960-12-31T11:12:13.000Z" ));

        testBoilerPlate( 'eoy_date_test', { 'eoy_date_test': 'datetimeoffset(3)' },

            function (done) {

                var insertQuery = "INSERT INTO eoy_date_test (eoy_date_test) VALUES (?);";
                conn.queryRaw( insertQuery, [ eoyDate ], function( e ) {

                    assert.ifError( e );
                    done();
                });
            },
            // test valid dates
            function( done ) {

                conn.queryRaw( "SELECT eoy_date_test FROM eoy_date_test", function( e, r ) {

                    assert.ifError( e );

                    var expectedDates = [];
                    var expectedDate = eoyDate;
                    expectedDates.push( [ expectedDate ]);
                    var expectedResults = { meta: [ { name: 'eoy_date_test', size: 30, nullable: true, type: 'date', sqlType: 'datetimeoffset' } ],
                        rows: expectedDates 
                    };
                    assert.deepEqual( expectedResults.meta, r.meta );
                    assert( r.rows.length == 1 );
                    for( var row in r.rows ) {
                        for( var d in row ) {
                            assert.deepEqual( expectedResults.rows[row][d], r.rows[row][d] );
                        }
                    }
                    done();
                })
            },
            test_done );
      });
  });

  test( 'verify Buffer objects as input parameters', function( test_done ) {

    sql.open( conn_str, function( err, conn ) {
        
        assert.ifError( err );
        var b = new Buffer( '0102030405060708090a', 'hex' );

        testBoilerPlate( 'buffer_param_test', { 'buffer_param' : 'varbinary(100)' },

            function( done ) {
                conn.queryRaw( "INSERT INTO buffer_param_test (buffer_param) VALUES (?)", [ b ], function( e, r ) {
                    assert.ifError( e );
                    done();
                });
            },
            function( done ) {
                conn.queryRaw( "SELECT buffer_param FROM buffer_param_test WHERE buffer_param = ?", [ b ], function( e, r ) {
                    assert.ifError( e );
                    assert( r.rows.length = 1 );
                    assert.deepEqual( r.rows[0][0], b );
                    done();
                });
            },
            test_done );
    });
  });

  test( 'verify buffer longer than column causes error', function( test_done ) {

    sql.open( conn_str, function( err, conn ) {

        assert.ifError( err );
        var b = new Buffer( '0102030405060708090a', 'hex' );

        testBoilerPlate( 'buffer_param_test', { 'buffer_param' : 'varbinary(5)' },

            function( done ) {
                conn.queryRaw( "INSERT INTO buffer_param_test (buffer_param) VALUES (?)", [ b ], function( e, r ) {
                    var expectedError = new Error('[Microsoft][SQL Server Native Client 11.0][SQL Server]String or binary data would be truncated.');
                    expectedError.sqlstate = "22001";
                    expectedError.code = 8152;
                    assert.deepEqual( e, expectedError );
                    done();
                });
            },
            function( done ) {
                done();
            },
            test_done );
    });
  });

  test( 'verify that non-Buffer object parameter returns an error', function( test_done ) {

    sql.open( conn_str, function( err, conn ) {

        assert.ifError( err );
        var o = { field1: 'value1', field2: -1 };

        testBoilerPlate( 'non_buffer_object', { 'object_col': 'varbinary(100)'},
            
            function( done ) {
                conn.queryRaw( "INSERT INTO non_buffer_object (object_col) VALUES (?)", [ o ], function( e, r ) {
                    assert( e == "Error: IMNOD: [msnodesql] Parameter 1: Invalid parameter type" );
                    done();
                });
            },
            function( done ) {
                done();
            },
            test_done );
    });
  });
});
