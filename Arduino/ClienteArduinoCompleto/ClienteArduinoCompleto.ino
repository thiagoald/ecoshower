#include <Ethernet.h>
#include <SPI.h>
#include <stdio.h>

#define SP_VCC    9          //sensor de presen�a
#define SP_TRIG   10
#define SP_ECHO   11
#define SP_GND    12
#define VALVULA   7          //valvula solenoide
#define SF_PULSO  2          //sensor de fluxo


#define ESPERA    1000       // Delay (em ms) entre um POST e outro
#define PORTA     3000       // Porta do servidor
#define BAUD_RATE 9600       // Taxa de comunicação serial

byte mac[]  = {  0x10, 0x1F, 0x74, 0x40, 0x7A, 0xD5 };  // Endere�o MAC do shield ethernet
byte ipArduino[]   = {192, 168, 1,0};               // IP est�tico do arduino (est� configurado para o IP do grad 1)
char dadosDoBanho[100];                                // String onde escrevemos os dados (no formato JSON) para mandar pro servidor
char ipServidor[30] = "192.168.1.48";                    // IP local da m�quina onde o servidor est� rodando
String resposta = "";
int estado = 0;
float vazao = 0;                                           // Variavel para armazenar o valor em L/min
float media=0;                                             // Variavel para tirar a media a cada 1 minuto
int contaPulso;                                            // Variavel para a quantidade de pulsos
int tempo= 30 * 100;
int validacaobanho = 0;
int cm=0; // tirar quando os componentes estiverem juntos
int contcheck = 0;
int banho = 0;
int timeout = 0;
int aux = 0;
int fim =1;


EthernetClient client;


void setup()
{
  //Ethernet.begin(mac);  // IP din�mico
  Ethernet.begin(mac,ipArduino);  // IP est�tico
  Serial.begin(BAUD_RATE);
  Serial.print("Usando o seguinte IP: ");
  Serial.println(Ethernet.localIP());
  //pinMode (SP_VCC,OUTPUT);                                // vcc
  //pinMode(SP_TRIG, OUTPUT);                               // TriG
  //pinMode (SP_ECHO, INPUT);                               // Echo
  //pinMode (SP_GND,OUTPUT);                                // GND
  //delay(2000);
  //pinMode(VALVULA,OUTPUT);
  //Serial.begin(9600);  
  //sensor de fluxo
  //pinMode(SF_PULSO, INPUT);
  //attachInterrupt(0, incpulso, RISING);                   // Configura o pino 2(Interrup��o 0) para trabalhar como interrup��o
  //Serial.println("\n\nInicio\n\n");                       // Imprime Inicio na serial
  Serial.println("Conectando a rede local...");
  while(!client.connect(ipServidor,PORTA))
  {
    Serial.println("Tentando Conectar Servidor");  
  } 
  Serial.println("Servidor Conectado Com Sucesso!");  
  
}


long microsecondsToInches(long microseconds)
{
  return microseconds / 74 / 2;
}
long microsecondsToCentimeters(long microseconds)
{
  return microseconds / 29 / 2;
}
void incpulso ()
{ 
  contaPulso++; //Incrementa a vari�vel de contagem dos pulsos
} 
String LerResposta ()
{
   
    while (client.available())
    {
      char c = client.read();
      if(c=='?')
      { 
        while(client.available())
        {
          fim = 1;
          char c = client.read();
          if(c=='?')
          {
            break;
          }
          resposta  += c;
        }
      }
      if(fim)
        break;
    }
    fim = 0;
    return resposta;
}

void enviar(char *dados)
{      
    Serial.println("Enviando POST ao servidor: ");
    client.println("POST / HTTP/1.1");          
    client.print("Host: ");
    client.println(ipServidor);
    client.println("Content-Type: application/json");
    client.println("Connection: open");
    client.println("User-Agent: Arduino/1.0");
    client.print("Content-Length: ");
    client.println(strlen(dados));
    client.println();
    client.print(dados);
    client.println();
    resposta = LerResposta();
    Serial.print("resposta : ");
    Serial.println(resposta);
    
}

void zera()
{
  estado = 0;
  vazao = 0;                                           // Variavel para armazenar o valor em L/min
  media=0;                                             // Variavel para tirar a media a cada 1 minuto
  contaPulso;                                            // Variavel para a quantidade de pulsos
  validacaobanho = 0;
  contcheck = 0;
  banho = 0;
  timeout = 0;
  aux =0;
}


void loop()                                           
{
  delay(1000);
  if(estado==0)
  { 
    sprintf(dadosDoBanho, "{\"nome\" : \"fulano\", \"duracao\":%d,\"Valor\":%d}", 10,2);     //Preparo estrutura a ser enviada, valor == 2 significa: pedindo permissao
    enviar(dadosDoBanho);                                 //Envio pedido de autenticacao
    if(resposta != "Work")                                //Enquando não houver liberacao do servidor, eu fico pedindo permissao (Pooling)
    {
      Serial.println("LIBERACAO NEGADA");                 // Esse if poderia ser um while ^^, veremos no futuro
      validacaobanho = 0;                                  // banho encerrado ou nem comecou
    }
    else                                                  //Servidor liberou
    {
      if(validacaobanho==0)                               // Se houve a liberacao do banho agora (não é uma checagem de rotina)
        zera;                                             //funcao que reseta as variaveis
      if(banho==1)                                        // Usuario estava com o chuveiro aberto (para o caso de não ser uma primeira liberacao)
        estado = 2;                                       // Va direto para o banho
      else                                                // Se ele nao estava tomando banho, so podia esta esperando o chuveiro abrir ( serve tanto para o caso de ser a primeira liberacao ou uma checagem de rotina)
        estado = 5;                                       // va para o estado de espera, Aki deveria ser estado 1, estado 5 para testes sem os componentes   

      Serial.println("LIBERACAO PERMITIDA");              // Printe uma mensagem pra informar que o banho comecara
    }
    
  }

  
  else if(estado == 5)
  {
    Serial.println("LIBERACAO PERMITIDA");                // Testes sem os componentes eletronicos
  }


  
  else if(estado == 1)
  {
      Serial.println("Esperando Usuario");
      validacaobanho = 1;                               //O banho pode ser considerado iniciado oficialmente
      digitalWrite(VALVULA,HIGH);                       //Fecha a valvula
      timeout++;                                        //Contando tempo para timeout
      delay(100);
      contcheck = contcheck + 100;                      //Contando tempo para atualizacao, eu preferi usar 2 contadores diferentes...
      if(timeout > tempo)                               //Se timeOut estoura
      {
         Serial.println("Banho cancelado");
         estado = 3;                                    //Estado 3 = atualizacao
      }
      else if(cm < 100)                                 //Cliente entra no alcance do chuveiro
         estado = 2;                                    //Estado 2 = inicio do banho
  }

  
  else if(estado == 2)
  {
      Serial.println("Banho Acontecendo");
      timeout = 0;                                      //tempo zerado para um nova recontagem
      digitalWrite(VALVULA,LOW);                        //abre a valvula
      contaPulso = 0;                                   //Zera a vari�vel para contar os giros por segundos
      sei();                                            //Habilita interrupcao
      delay (1000);                                     //Aguarda 1 segundo
      contcheck = contcheck + 1000;
      cli();                                            //Desabilita interrupcao
      vazao += contaPulso/(6.12*60);                    //Converte para L/s
      media=media+vazao;                                //Soma a vazao para o calculo da media 
      Serial.print("Vazao: ");
      Serial.println(vazao);                            //Imprime na serial o valor da vazao
      banho = 1;
      if(cm > 100)                                      //Se o usuario sair do alcancedo chuveiro
      {
          banho = 0;
          Serial.println("Banho Parado");
          digitalWrite(VALVULA,HIGH);                    //Fecha a valvula
          estado = 0;                                    //Vai para autenticacao
      }
  }

  
  else if(estado == 3)
  {
      sprintf(dadosDoBanho, "{\"nome\" : \"fulano\", \"duracao\":%d,\"Valor\":%d}", 10,1);      // Valor ==1 significa: Atualize o servidor
      enviar(dadosDoBanho);
      if(timeout > tempo)
      {
         sprintf(dadosDoBanho, "{\"nome\" : \"fulano\", \"duracao\":%d,\"Valor\":%d}", 10,3);   // Valor ==3 significa: Foi timeout, avise o servidor
         enviar(dadosDoBanho);
         delay(1000);                                                                           // Tempo para o servidor processar o que aconteceu
      }
      else                                                                                      // Nesse caso, não foi time out e sim uma atualizacao de rotina
         contcheck = 0;                                                                         // Zere o contador para a proxima contagem
      estado=0;                                                                                 // va para o estagio de autenticacao
  }

  if(contcheck>=1000)                                                                            // Checo se o tempo para autenticar virou
  {
    aux=aux+1;                                                                                    //variavel auxiliar que me ajuda a saber quanto tempo se passou entres as autenticacoes
    if(aux==5)                                                                                   // Houveram 5 autenticacos = 5 segundos = Atualizacao
    {
      aux=0;
      estado = 3;                                                                                //Atualize
    }
    else
      estado = 0;                                                                                //Autentique
  }    
  resposta = "";
}



  



