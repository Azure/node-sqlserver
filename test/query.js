// query.js
// test rudimentary queries for mssql

var sql = require('mssql');
var assert = require( 'assert' );

suite( 'query', function() {

    var conn_str = "Driver={SQL Server Native Client 11.0};Server={.\\SQL2008R2};Trusted_Connection={Yes};Database=AdventureWorks2008R2";

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

    test('simple query of tables like Product%', function( done ) {

        var like = 'Product%';

        sql.query( conn_str, "SELECT name FROM sys.tables WHERE name LIKE ?", [like], function ( err, results ) {

            assert.ifError( err );
            
            var expected = [ { 'name': 'ProductInventory' },
                             { 'name': 'ProductListPriceHistory' },
                             { 'name': 'ProductModel' },
                             { 'name': 'ProductModelIllustration' },
                             { 'name': 'ProductModelProductDescriptionCulture' },
                             { 'name': 'ProductPhoto' },
                             { 'name': 'ProductProductPhoto' },
                             { 'name': 'ProductReview' },
                             { 'name': 'ProductSubcategory' },
                             { 'name': 'ProductVendor' },
                             { 'name': 'Product' },
                             { 'name': 'ProductCategory' },
                             { 'name': 'ProductCostHistory' },
                             { 'name': 'ProductDescription' },
                             { 'name': 'ProductDocument' } ];

            assert.deepEqual( results, expected, "tables like Product% not equal" );

            done();
        });

    });

/*
    test( 'streaming test', function( done ) {

        var results = sql.query( conn_str, 'select name, modify_date from sys.tables order by modify_date' );

        results.on('meta', function(meta) { console.log({meta: meta}); });
        results.on('row', function(idx) { console.log({row: idx}); });
        results.on('column', function(idx, data, more) { console.log({column: idx, data: data, more: more}); });
        results.on('done', function() { done(); });
        results.on('error', function(err) { console.log({error: err}); });
    });
*/

});
