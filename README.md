# Note

This driver is a native C driver that uses the Microsoft ODBC driver under the hood. If you are looking for a pure javascript driver with no dependencies please check out [Tedious](https://github.com/pekim/tedious).

# Microsoft Driver for Node.js for SQL Server

The Microsoft Driver for Node.js for SQL Server allows Node.js applications on
Microsoft Windows and Microsoft Windows Azure to access Microsoft SQL Server 
and Microsoft Windows Azure SQL Database.

This is an initial preview release, and is not production ready. We welcome any
feedback, fixes and contributions from the community at this very early stage.

## Prerequisites

The following prerequisites are necessary prior to using the driver:

* Node.js - use node.js version 0.8.x
* [Python 2.7](https://www.python.org/download/releases/2.7/)
* [Visual C++ 2010 Express](https://app.vssps.visualstudio.com/profile/review?download=true&amp;family=VisualStudioCExpress&amp;release=VisualStudio2010&amp;type=web&amp;slcid=0x409&amp;context=eyJwZSI6MSwicGMiOjEsImljIjoxLCJhbyI6MCwiYW0iOjEsIm9wIjpudWxsLCJhZCI6bnVsbCwiZmEiOjAsImF1IjpudWxsLCJjdiI6OTY4OTg2MzU1LCJmcyI6MCwic3UiOjAsImVyIjoxfQ2)
 

Next install the msnodesql driver

1. Download the msnodesql driver by using the command ‘npm install msnodesql’.  This will install Microsoft’s NodeJS-SQL driver into your project. You should now have a folder called msnodesql inside node_modules. Note: This will only work with Node 0.8.9. If you had newer versions, you will have to downgrade.


2. Next go to the file explorer and naviage to the the C:\NodeJSSampleProject\node_modules\msnodesql folder and run the executable(.msi file). Note: You will need Visual C++ 2010 Express before you run the executable. This can be downloaded from [here](https://app.vssps.visualstudio.com/profile/review?download=true&amp;family=VisualStudioCExpress&amp;release=VisualStudio2010&amp;type=web&amp;slcid=0x409&amp;context=eyJwZSI6MSwicGMiOjEsImljIjoxLCJhbyI6MCwiYW0iOjEsIm9wIjpudWxsLCJhZCI6bnVsbCwiZmEiOjAsImF1IjpudWxsLCJjdiI6OTY4OTg2MzU1LCJmcyI6MCwic3UiOjAsImVyIjoxfQ2). 


## Test

Included are a few unit tests.  They require mocha, async, and assert to be 
installed via npm.  Also, set the variables in test-config.js, then run the 
tests as follows:

    cd test
    node runtests.js

## Known Issues

We are aware that many features are still not implemented, and are working to
update these. Please visit the [project on Github][project] to view 
outstanding [issues][issues].

## Usage

For now, please see the unit tests for usage examples.

## Contribute Code

If you would like to become an active contributor to this project please follow the instructions provided in [the Contribution Guidelines][contribute].

## License

The Microsoft Driver for Node.js for SQL Server is licensed under the Apache
2.0 license.  See the LICENSE file for more details.

[visualstudio]: http://www.microsoft.com/visualstudio/

[sqlncli]: http://www.microsoft.com/en-us/download/details.aspx?id=29065

[project]: https://github.com/windowsazure/node-sqlserver

[issues]: https://github.com/windowsazure/node-sqlserver/issues

[contribute]: https://github.com/WindowsAzure/node-sqlserver/blob/master/CONTRIBUTING.md




