var express = require('express');
var app = express();
var bodyParser = require('body-parser')
var fs = require('fs');
var dados = 'dados.txt'
var jsonDateStart 
var jsonDateEnd
var cont = 0
var ready = 0;
var __rootDir = 'C:/Users/Magda/Downloads/Servidor';

// Atencao
// 1. O servidor funciona asim: Se existe um txt com seu nome na pasta, estao, voce esta cadastrado (Gambiarra temporaria, ou nao).
// 2. Como nao sei fazer a autenticacao ainda, o servidor esta alheio a qualquer informacao enviada
// 3. Se o json vem com Valor == 1, isso é uma atualizacao
// 4. Se o json vem com Valor == 0, o arduino esta pedindo permissao para comecar
// 5. O servidor ate pode funcionar assim, mas, precisa sinalizar ao arduino para inicar o banho (nao consegui fazer)
//Fluxo de controle: O arduino pede permissao ao servidor para iniciar o banho, o servidor permite ou nao o banho, o arduino tendo a confirmacao do banhos
//inicia o banho e no final manda a atualizacao.
app.use(express.static('public'));
app.use( bodyParser.json() );
app.use(bodyParser.urlencoded({     // to support URL-encoded bodies
  extended: true
}));

// Add headers
app.use(function (req, res, next) {

    // Website you wish to allow to connect
    res.setHeader('Access-Control-Allow-Origin', 'http://localhost:3000/data');

    // Request methods you wish to allow
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS, PUT, PATCH, DELETE');

    // Request headers you wish to allow
    res.setHeader('Access-Control-Allow-Headers', 'X-Requested-With,content-type');

    // Set to true if you need the website to include cookies in the requests sent
    // to the API (e.g. in case you use sessions)
    res.setHeader('Access-Control-Allow-Credentials', true);

    // Pass to next layer of middleware
    next();
});


app.get('/login', function (req, res)
{
	res.sendFile('login.html', {root: __rootDir});
});

app.get('/estatisticas', function (req, res)
{
	res.sendFile('estatisticas.html', {root: __rootDir});
});

app.get('/usuarioAtual', function (req, res)
{
	fs.readFile('./Usuarios/usuarioAtual.txt', 'utf8', function(err,data){
			//console.log(req.body.Valor);	
		if(err){
			res.send('erro');
		}	
		else
		{
			console.log("Usuario atual:" + data);
			res.send(data);
		}
	});
});


app.get('/data', function (req, res)
{
	console.log("Nome do cara: " + req.query.nome);
	fs.readFile('./Usuarios/'+req.query.nome+'.txt', 'utf8', function(err,data){
			//console.log(req.body.Valor);	
		if(err){
			console.log('Cliente Nao Cadastrado, Porfavor, tente novamente');
			res.send('erro');
		}	
		else
		{
			var resposta = [];
			var dadosArquivo = JSON.parse(data);
			// for(var i=0; i<dadosArquivo.banhos.length; i++){
			// 	resposta.push(dadosArquivo.banhos[i].consumo);
			// }
			console.log(dadosArquivo);
			res.send(JSON.stringify(dadosArquivo));
		}
	});
});

app.get('/ready', function(req,res)
{
	console.log("Recebi GET no /ready pela " + ready + " vez!");
	ready++;
	fs.readFile('./Usuarios/usuarioAtual.txt', 'utf8', function(err,nomeDoUsario){
		if(err){
			console.log('Erro fatal!');
			res.send("?ninguem?");
		}
		else
		{
			console.log('Usuario atual:' + nomeDoUsario);
			res.send('?'+nomeDoUsario+'?');
		}
	});
});



app.post('/', function (req, res) {
	console.log("Recebi POST!");
	var now = new Date();
	var string = '?' ;
	//res.send(string + 'sucesso'); // Confirma recebimento
    cont = cont + 1;
    if(cont<4)
    	res.send(string + 'NWork' + string);
    else
    	res.send(string + 'Work' + string);

    var usuarioAtual;

    fs.readFile('./Usuarios/usuarioAtual.txt', 'utf8', function(err,nomeDoUsario){
		if(err){
			console.log('Erro fatal!');
			res.send("?ninguem?");
		}
		else
		{
			console.log('Usuario atual:' + nomeDoUsario);
			usuarioAtual = nomeDoUsario;
			console.log("lendo arquivo: " + './Usuarios/'+usuarioAtual+'.txt');
		    fs.readFile('./Usuarios/'+usuarioAtual+'.txt', 'utf8', function(err,data){	 

		 				//console.log(req.body.Valor);	
		 				if(err)
							console.log('Cliente Nao Cadastrado, Porfavor, tente novamente');	
						else
						{
							//console.log('Recebido');

		    				if (req.body.Valor==0 )  				 // Meu cliente esta pedindo permissao para comecar, assim que eu permitir: 
							{										 // Valor igual a 0 quer dizer que o banho ainda nao se iniciou
								now = new Date()
								jsonDateStart = now.toJSON();								// Pego a hora de inicio do banho
								console.log('Cliente Cadastrado,Aproveite Seu banho');	    // Banho inicia
								//Retorna Sinal Positivo
							}
		    				else if(req.body.Valor==1)				//O arduino ja esta iniciando o banho, logo, quando ele vai atualizar as informacoes
		    				{										//do cliente, ele coloca Valor ==1
								jsonDateEnd = now.toJSON();         //Data do final do banho
		  	  					var dadosArquivo = data;

		    					if(data=='')						//Caso Arquivo esteja Vazio (Primeiro Banho)
		    					{
		    						console.log('Primeira insercao na data ' + jsonDateEnd);
		    						fs.writeFile('./Usuarios/'+usuarioAtual+'.txt', JSON.stringify({'nome':usuarioAtual ,'banhos':[{'inicio':jsonDateStart,'fim':jsonDateEnd,'consumo':req.body.Consumo,'fluxoMedio':req.body.FluxoMedio,'duracao':req.body.Duracao}]}));
		    						console.log('Inserido Com Sucesso');
								}
								else								//Caso Nao seja o primeiro banho (Apenas atualizacao)
								{
									now = new Date();
									var jsonDateEnd = now.toJSON();
									var dadosArquivo1 = JSON.parse(data);
									console.log('Acrescentando na data' + jsonDateEnd);
									dadosArquivo1.banhos[dadosArquivo1.banhos.length-1].consumo.push(parseInt(req.body.Consumo)) ;
									fs.writeFile('./Usuarios/'+usuarioAtual+'.txt',JSON.stringify(dadosArquivo1));
									console.log('Inserido Com Sucesso');
								}
								console.log('Atualizado com Sucesso');
							}		
						}
			});
		}
	});


    //console.log('Cliente Nao Cadastrado, Porfavor, tente novamente');	
    

});


var server = app.listen(3000, function () {
  var host = server.address().address;
  var port = server.address().port;
  console.log('Example app listening at http://%s:%s', host, port);
});