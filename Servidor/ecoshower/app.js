var express = require('express');
var app = express();
var bodyParser = require('body-parser')
var fs = require('fs');
var dados = 'dados.txt'
var autenticado =1


app.use( bodyParser.json() );
app.use(bodyParser.urlencoded({     // to support URL-encoded bodies
  extended: true
}));


app.get('/', function (req, res) {

	if(autenticado)
	{
		fs.readFile(dados, 'utf8', function(err,data){	
				res.send(data);
		});
	}
	else
		res.send('Falha Na Autenticacao');
});




app.post('/', function (req, res) {
	//console.log(req.body);
	//console.log(req.body.Valor);
	console.log('post');
	var now = new Date();
	var jsonDate = now.toJSON();

	//fs.writeFile(dados, JSON.stringify(req.body));

    fs.readFile('./Usuarios/'+req.body.nome+'.txt', 'utf8', function(err,data){	
     			var dadosArquivo = JSON.parse(data);
     			dadosArquivo.banhos.push({'duracao':req.body.duracao, 'fluxoMedio':req.body.fluxoMedio,'consumo':req.body.consumo});
     			fs.writeFile('./Usuarios/'+req.body.nome+'.txt', JSON.stringify(dadosArquivo));
     			console.log(JSON.stringify(dadosArquivo));
				res.send(data);
	});


	//var arquivo = require ('./fulano.txt');
	//var arquivo = require ('./Usuarios/'+req.body.nome+'.txt');
	//console.log(JSON.stringify(arquivo));
	//fs.writeFile(dados, req.body.Valor);
	
	

	
});


var server = app.listen(3000, function () {
  var host = server.address().address;
  var port = server.address().port;
  console.log('Example app listening at http://%s:%s', host, port);
});