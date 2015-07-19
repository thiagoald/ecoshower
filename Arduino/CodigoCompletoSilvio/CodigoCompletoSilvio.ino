#include <Ethernet.h>
#include <SPI.h>
#include <stdio.h>

#define SP_VCC    4//sensor de presença
#define SP_TRIG   5
#define SP_ECHO   6
#define SP_GND    7
#define VALVULA   8//valvula solenoide
#define SF_PULSO  2//sensor de fluxo



#define ESPERA    1000       // Delay (em ms) entre um POST e outro
#define PORTA     3000       // Porta do servidor
#define BAUD_RATE 9600       // Taxa de comunicação serial

byte mac[]  = {  0x10, 0x1F, 0x74, 0x40, 0x7A, 0xD5 };     // Endere?o MAC do shield ethernet
byte ipArduino[]   = {172, 20, 18,251};                      // IP est?tico do arduino (est? configurado para o IP do grad 1)
char dadosDoBanho[100];                                    // String onde escrevemos os dados (no formato JSON) para mandar pro servidor
char ipServidor[30] = "172.20.18.24";                     // IP local da m?quina onde o servidor est? rodando
String resposta = "";
int estado = 0;
float vazao = 0;                                           // Variavel para armazenar o valor em L/min
float media=0;                                             // Variavel para tirar a media a cada 1 minuto
int contaPulso;                                            // Variavel para a quantidade de pulsos
int tempo= 30 * 100;
int validacaobanho = 0;
int contcheck = 0;
int banho = 0;
int timeout = 0;
int aux = 0;
int fim =1;



EthernetClient client;


void setup()
{
  //Ethernet.begin(mac);  // IP din?mico
  Ethernet.begin(mac,ipArduino);  // IP est?tico
  Serial.begin(BAUD_RATE);
  Serial.print("Usando o seguinte IP: ");
  Serial.println(Ethernet.localIP());
  pinMode (SP_VCC,OUTPUT);                                // vcc
  pinMode(SP_TRIG, OUTPUT);                               // TriG
  pinMode (SP_ECHO, INPUT);                               // Echo
  pinMode (SP_GND,OUTPUT);                                // GND
  delay(2000);
  pinMode(VALVULA,OUTPUT);
  Serial.begin(9600);  
  
  pinMode(SF_PULSO, INPUT);
  attachInterrupt(0, incpulso, RISING);                   // Configura o pino 2(Interrup??o 0) para trabalhar como interrup??o
  //Serial.println("\n\nInicio\n\n");                       // Imprime Inicio na serial
  Serial.println("Conectando a rede local...");
  //while(!client.connect(ipServidor,PORTA))
  //{
  //  Serial.println("Tentando Conectar Servidor");  
  //} 
  //Serial.println("Servidor Conectado Com Sucesso!");  
  
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
  contaPulso++; //Incrementa a vari?vel de contagem dos pulsos
} 
String LerResposta ()
{
   
    while (client.available()||resposta=="")
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
      if(fim&&resposta!="")
        break;
    }
    fim = 0;
    return resposta;
}

void enviar(char *dados)
{      
    while(!client.connect(ipServidor,PORTA));
    Serial.println("Enviando GET ao servidor: ");
    client.println("GET /ready HTTP/1.1");        
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
    client.stop();
    
}


void loop()                                           
{
  digitalWrite(SP_VCC, HIGH);
  long duration, inches, cm;
  digitalWrite(SP_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(SP_TRIG, HIGH);
  delayMicroseconds(5);
  digitalWrite(SP_TRIG, LOW);
  duration = pulseIn(SP_ECHO, HIGH);
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
  if(estado==0)
  { 
    sprintf(dadosDoBanho, "{\"nome\" :  \"fulano\", \"duracao\":%d, \"consumo\":%d, \"Valor\":%d}", 10,20,3);     //Preparo estrutura a ser enviada, valor == 2 significa: pedindo permissao
    enviar(dadosDoBanho);                                 //Envio pedido de autenticacao
    if(resposta == "ninguem" || resposta == "")            //Enquando não houver liberacao do servidor, eu fico pedindo permissao (Pooling)
    {
      Serial.println("LIBERACAO NEGADA");                 // Esse if poderia ser um while ^^, veremos no futuro
      validacaobanho = 0;                                  // banho encerrado ou nem comecou
    }
    else                                                  //Servidor liberou
    {
        estado = 1;
    }
    
  }
  
  else if(estado == 1)
  {
      Serial.println("Esperando Usuario");
      
      digitalWrite(VALVULA,HIGH);                       //Fecha a valvula
      timeout++;                                        //Contando tempo para timeout
      delay(100);
      if(timeout > tempo)                               //Se timeOut estoura
      {
         Serial.println("Banho cancelado");
         estado = 3;                                    //Estado 3 = atualizacao
      }
      if(microsecondsToCentimeters(duration) < 100)                                 //Cliente entra no alcance do chuveiro
         estado = 2;                                    //Estado 2 = inicio do banho
  }

  
  else if(estado == 2)
  {
      
      Serial.print("Banho Acontecendo com Cm: ");
      Serial.println(microsecondsToCentimeters(duration));
      timeout = 0;                                      //tempo zerado para um nova recontagem
      digitalWrite(VALVULA,LOW);                        //abre a valvula
      contaPulso = 0;                                   //Zera a vari?vel para contar os giros por segundos
      sei();                                            //Habilita interrupcao
      delay (1000);                                     //Aguarda 1 segundo
      cli();                                            //Desabilita interrupcao
      vazao += contaPulso/(6.12*60);                    //Converte para L/s
      media=media+vazao;                                //Soma a vazao para o calculo da media 
      Serial.print("Vazao: ");
      Serial.println(vazao);                            //Imprime na serial o valor da vazao
     
      if(microsecondsToCentimeters(duration) > 100)                                      //Se o usuario sair do alcancedo chuveiro
      {
          
          Serial.println("Banho Parado");
          digitalWrite(VALVULA,HIGH);                    //Fecha a valvula
          estado = 1;                                    //Vai para autenticacao
      }
  }

  
  else if(estado == 3)
  {
         sprintf(dadosDoBanho, "{\"nome\" : \"fulano\", \"duracao\":%d,\"Valor\":%d}", 10,3);   // Valor ==3 significa: Foi timeout, avise o servidor
         enviar(dadosDoBanho);
         delay(1000);                                                                           // Tempo para o servidor processar o que aconteceu
                                                            // va para o estagio de autenticacao
  }

  delay(100);
  resposta = "";
}
