// connect.js
// test suite for connections

var sql = require('../');
var assert = require( 'assert' );
var config = require( './test-config' );

suite( 'open', function() {

    test('trusted connection to a server', function( done ) {

        sql.open( "Driver={SQL Server Native Client 11.0};Server=" + config.server + ";Trusted_Connection={Yes}", 
                  function( err, conn ) {

                      assert.ifError( err );
                      assert( typeof conn == 'object');

                      done();
                  });
    });
});

