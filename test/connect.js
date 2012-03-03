// connect.js
// test suite for connections

var sql = require('mssql');
var assert = require( 'assert' );

suite( 'open', function() {

    test('trusted connection to a server', function( done ) {

        sql.open( "Driver={SQL Server Native Client 11.0};Server=.\\SQL2008R2;Database=test;Trusted_Connection={Yes}", 
                  function( err, conn ) {

                      assert.ifError( err );
                      assert( typeof conn == 'object');

                      done();
                  });
    });
});

