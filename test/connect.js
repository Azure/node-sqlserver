//---------------------------------------------------------------------------------------------------------------------------------
// File: connect.js
// Contents: test suite for connections
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

var sql = require('../');
var assert = require( 'assert' );
var config = require( './test-config' );

suite( 'open', function() {

    test('trusted connection to a server', function( done ) {

        sql.open(config.conn_str, 
                  function( err, conn ) {

                      assert.ifError( err );
                      assert( typeof conn == 'object');

                      done();
                  });
    });

    test('verify closed connection throws an exception', function( done ) {

    	sql.open( config.conn_str, function( err, conn ) {

			assert.ifError( err );

			conn.close();
			var thrown = false;

			try {
				conn.query( "SELECT 1", function( err, results ) {

					assert.ifError( err )

				});
			}
			catch( e ) {

				assert( e == "Error: [msnodesql] Connection is closed.")
				thrown = true;
			}

			assert( thrown );
			done();
    	});
    });

    test( 'verify connection is not closed prematurely until a query is complete', function( done ) {

		sql.open( config.conn_str, function( err, conn ) {

		  assert.ifError( err );

		  var closeCalled = false;
		  var stmt = conn.queryRaw( "select 1" );

		  stmt.on( 'meta', function( m ) {  });
		  stmt.on( 'done', function( ) { assert( closeCalled ); done(); });
		  stmt.on( 'column', function( c, d ) { assert( c == 0 && d == 1 ); });
		  stmt.on( 'error', function( e ) { assert.ifError( e ); });
		  stmt.on( 'row', function( r ) { assert( r == 0 ); conn.close(); closeCalled = true; });
		});
	});

	test( 'verify that close immediately flag only accepts booleans', function( done ) {

    	sql.open( config.conn_str, function( err, conn ) {

			assert.ifError( err );

			var thrown = false;

			try {
				conn.close( "SELECT 1", function( err ) {

					assert.ifError( err )
				});
			}
			catch( e ) {

				assert( e == "Error: [msnodesql] Invalid parameters passed to close.")
				thrown = true;
			}

			conn.close();
			assert( thrown );
			done();
    	});

	});
});

