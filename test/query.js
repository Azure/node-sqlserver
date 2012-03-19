// query.js
// test rudimentary queries for mssql

var sql = require('../');
var assert = require( 'assert' );
var config = require( './test-config' );

suite( 'query', function() {

    var conn_str = "Driver={SQL Server Native Client 11.0};Server=" + config.server + ";Trusted_Connection={Yes};";

    test('simple query', function( done ) {

        sql.query(conn_str, "SELECT 1 as X, 'ABC', 0x0123456789abcdef ", function( err, results ) {

            assert.ifError( err );

            var buffer = new Buffer('0123456789abcdef', 'hex');
            var expected = [ { 'X': 1, 'Column1' : 'ABC', 'Column2': buffer } ];
            
            assert.deepEqual( results, expected, "Results don't match");

            done();
        });
    });

    test('simple raw query', function( done ) {

        sql.queryRaw(conn_str, "SELECT 1 as X, 'ABC', 0x0123456789abcdef ", function( err, results ) {

            assert.ifError( err );

            var buffer = new Buffer('0123456789abcdef', 'hex');

            var expected = { meta:
                             [ { name: 'X', size: 10, nullable: false, type: 'number' },
                               { name: '', size: 3, nullable: false, type: 'text' },
                               { name: '', size: 8, nullable: false, type: 'binary' } ],
                             rows: [ [ 1, 'ABC', buffer ] ] }

            assert.deepEqual( results, expected, "raw results didn't match" );

            done();
        });

    });

    test('simple query of types like var%', function( done ) {

        var like = 'var%';

        sql.query( conn_str, "SELECT name FROM sys.types WHERE name LIKE ?", [like], function ( err, results ) {

            assert.ifError( err );
            
            var expected = [ { 'name': 'varbinary' },
                             { 'name': 'varchar' } ];

            assert.deepEqual( results, expected, "types like var% not equal" );

            done();
        });

    });

    test( 'streaming test', function( done ) {

        var like = 'var%';
        var expected =[ [ { name: 'name', size: 128, nullable: false, type: 'text' } ],
                        { row: 0 },
                        { column: 0, data: 'varbinary', more: false },
                        { row: 1 },
                        { column: 0, data: 'varchar', more: false } ];

        var stmt = sql.query( conn_str, 'select name FROM sys.types WHERE name LIKE ?', [like] );
        var results = [];

        stmt.on('meta', function(meta) { results.push( meta ); });
        stmt.on('row', function(idx) { results.push( { row: idx } ); });
        stmt.on('column', function(idx, data, more) { results.push( { column: idx, data: data, more: more }); });
        stmt.on('done', function() { assert.deepEqual( expected, results ); done(); });
        stmt.on('error', function(err) { assert.ifError( err ); });
    });

    test( 'serialized queries', function( done ) {

        var expected = [ { meta: [ { name: '', size: 10, nullable: false, type: 'number' } ],
                           rows: [ [ 1 ] ] },
                         { meta: [ { name: '', size: 10, nullable: false, type: 'number' } ],
                           rows: [ [ 2 ] ] },
                         { meta: [ { name: '', size: 10, nullable: false, type: 'number' } ],
                           rows: [ [ 3 ] ] },
                         { meta: [ { name: '', size: 10, nullable: false, type: 'number' } ],
                           rows: [ [ 4 ] ] },
                         { meta: [ { name: '', size: 10, nullable: false, type: 'number' } ],
                           rows: [ [ 5 ] ] } ];

        var results = [];

        var c = sql.open( conn_str, function( e ) {

            assert.ifError( e );

            c.queryRaw( "SELECT 1", function( e, r ) {

                assert.ifError( e );

                results.push( r );
            });

            c.queryRaw( "SELECT 2", function( e, r ) {

                assert.ifError( e );

                results.push( r );
            });

            c.queryRaw( "SELECT 3", function( e, r ) {

                assert.ifError( e );

                results.push( r );
            });

            c.queryRaw( "SELECT 4", function( e, r ) {

                assert.ifError( e );

                results.push( r );
            });

            c.queryRaw( "SELECT 5", function( e, r ) {

                assert.ifError( e );

                results.push( r );

                assert.deepEqual( expected, results );
                done();
            });
        });
    });

});
