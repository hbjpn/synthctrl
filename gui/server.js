var argv = require('optimist').argv;
var cp = require('child_process');
var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io').listen(http);
var session = require('express-session');
var bodyParser = require("body-parser")
  , cookieParser = require("cookie-parser")
  , methodOverride = require('method-override');
// Set up the Session middleware using a MongoDB session store
var expressSession = require("express-session");
var sessionMiddleware = expressSession({
    name: "sessioncookie",
    secret: "hogefugahoge",
    store: new expressSession.MemoryStore()
});


var port = argv.port || 8081;
var datadir = argv.datadir || "../data/";
// Authentication for Facebook

console.log(argv);
app.set("view engine", "ejs");
app.use(sessionMiddleware);
app.use(cookieParser());
app.use(bodyParser());
app.use(methodOverride());


app.get('/', function(req, res){
  res.render('index');
});
function remove(array, val)
{
	for(var i = 0; i < array.length; ++i){
		if(array[i] == val){
			array.splice(i,1);
			break;
		}
	}
}

app.use(express.static(__dirname));
app.get("/test", function(req,res){
	res.send(req.user);
});

http.listen(port);
console.log("Listen on port " + port);

rooms = {};
contentList = {};


var fs = require("fs");



var net = require('net');

var HOST = '127.0.0.1';
var PORT = 6969;

var sockclient = new net.Socket();
sockclient.connect(PORT, HOST, function() {

    console.log('CONNECTED TO: ' + HOST + ':' + PORT);
    // Write a message to the socket as soon as the client is connected, the server will receive it as message from the client 
    //client.write('I am Chuck Norris!');

});

// Add a 'data' event handler for the client socket
// data is what the server sent to this socket
sockclient.on('data', function(data) {
    
    console.log('DATA: ' + data);
    // Close the client socket completely
    sockclient.destroy();
    
});

// Add a 'close' event handler for the client socket
sockclient.on('close', function() {
    console.log('Connection closed');
});


function GUID(){
	var streamId = "";
	for(var i = 0; i < 4; ++i){
		var s = ("00000000" + (parseInt(Math.floor(Math.random()*0xFFFFFFFF)).toString(16))).slice(-8);
		streamId += s;
	}
	return streamId;
}

RPCBody = {
	'list' : function(socket, data, res){

		fs.readdir(datadir, function(err, files){
				if (err) throw err;
				var fileList = [];
				files.filter(function(file){
					var path = datadir + "/" + file;
					return fs.statSync(path).isFile() && /.*\.json$/.test(file); //絞り込み
					}).forEach(function (file) {
						fileList.push(file);
						});
				console.log(fileList);
				res.send({error:false, files:fileList}); 
				});
	},
	'load' : function(socket, data, res){

		fs.readFile(datadir+"/"+data, "utf8", function(err, text){
				if (err) throw err;
				res.send({error:false, name:data, content:text}); 
				});
	},
	'save' : function(socket, data, res){

		fs.writeFile(datadir+"/"+data.name, data.content, function(err){
				if (err) res.send({error:true,msg:err});
				else res.send({error:false});
				});

	},
	'deploy' : function(socket, data, res){
		console.log("kitaaa");
		//res.send({error:false, msg:"hoge"});
		//return;
		cp.exec('../synthctrl '+ datadir+"/"+data.name,  function(err, stdout, stderr){
		//cp.exec('ls',  function(err, stdout, stderr){
			console.log(err);
			console.log(stdout);
			console.log(stderr);
			if(err) res.send({error:true, msg:err});
			else res.send({error:false, stdout:stdout, stderr:stderr});
		});	
	},

	'setprof' : function(socket, data, res){
		console.log('setprof : ' + JSON.stringify(data));
		var user_id = socket.request.session.passport.user;
		mongo.db.collection('users').update({user_id:user_id}, {user_id:user_id,profile:data}, {upsert:true}, function(err,ret){ console.log([err,ret]);});
		res.send({error:false});
	}
};

function RPCResponse(socket, rpcid)
{
	this.socket = socket;
	this.rpcid = rpcid;
	this.sent = false;
}

RPCResponse.prototype.send = function(resbody){
	if(!this.sent)
		this.socket.emit('RPC', {RPCID:this.rpcid,error:false,data:resbody});
	this.sent = true;
};

function cleanup()
{
	console.log("Graceful shutdown ... ");
	mongo.db.collection('rooms').update({},{$set:{member:[], content_list:{}}},{multi:true}, function(err,res){
		console.log("Clear member and content list from database done");
		process.exit(0);
	});
}

process.on('SIGTERM', cleanup);
process.on('SIGINT', cleanup);

io.sockets.
  use(function(socket,next){ sessionMiddleware(socket.request, {}, next); }).
  on('connection', function (socket) {
    console.log("Socket user id = " + JSON.stringify(socket.request.session.passport));
  socket.custom = {};
  socket.on('RPC', function(req) {
	  console.log('RPC call');
	  console.log(req);
	 if(req.evt in RPCBody){
		var res = new RPCResponse(socket, req.RPCID);
		 RPCBody[req.evt](socket, req.data, res);
	 }else{
		 socket.emit('RPC',{RPCID:req.RPCID,error:true});
	 }
  });
  socket.on('disconnect', function (){
  });
});

