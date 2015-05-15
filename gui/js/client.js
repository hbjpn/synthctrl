var app = angular.module("synthctrl",[]);
var fileListCtrl = null;
app.controller('FileList', function($scope){
	$scope.files = ["hoge.json","fuga.json", "poko.json"];	
	console.log($scope.files);
	fileListCtrl = $scope;
});

var man = null;

function run()
{
	alert("run");
}

function RPCManager(wsurl)
{
	this.srv_socket = io.connect(wsurl);
	this.SocketIOEvt = {};
	this.RPCS = {};
	this.RPCID = 0;	
}
RPCManager.prototype.RPC = function(evt, data, callback, options){
	var me = this;
	if(!('RPC' in me.SocketIOEvt)){
		me.srv_socket.on('RPC', function(res){
			me.RPCS[res.RPCID](res.data);
			delete me.RPCS[res.RPCID];
		});
		me.SocketIOEvt['RPC'] = true;
	}
	me.RPCS[me.RPCID] = callback;
	var req = {RPCID:me.RPCID, evt:evt, data:data};
	me.srv_socket.emit('RPC', req);
	me.RPCID++;
};

/**
 * Broadcasting based RPC. (Utility)
 */
RPCManager.prototype.on = function(evt, callback, options)
{
	var me = this;
	if(!(evt in me.SocketIOEvt)){
		me.srv_socket.on(evt, callback);
		me.SocketIOEvt[evt] = true;
	}
};

$("document").ready(function(){
	var man = new RPCManager("192.168.11.7:8081");

	man.RPC('list', {}, function(res){
		fileListCtrl.files = res["files"];
		fileListCtrl.$apply();		
	});	
});
