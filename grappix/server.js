
//var http = require('http');
var connect = require('connect');
var server = connect.createServer();
server.use(connect.static(__dirname + "/html"));


server.listen(8080);