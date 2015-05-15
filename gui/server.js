var argv = require('optimist').argv;
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

function leaveroom(socket)
{
	if(socket.custom.room){
		socket.broadcast.to(socket.custom.room).emit('peer_left',{peer:socket.custom.peer_id});
		var room_id = socket.custom.room;
			socket.leave(room_id);
		mongo.db.collection('rooms').update({room_id:room_id},{$pull: {"member" : socket.custom.peer_id}}, function(err,item){
			
		});
		//remove(rooms[room_id].member, socket.custom.peer_id);
			//if(rooms[room_id].member.length == 0){
		//	setDeleteTimer(room_id);
		//}
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

