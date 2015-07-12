#include <Ethernet.h>
#include <SPI.h>
#include <stdio.h>

#define ESPERA    1000   // Delay (em ms) entre um POST e outro
#define PORTA     3000   // Porta do servidor
#define BAUD_RATE 9600   // Taxa de comunica��o serial

byte mac[]  = {  0x10, 0x1F, 0x74, 0x40, 0x7A, 0xD5 };  // Endere�o MAC do shield ethernet
byte ipArduino[]   = {  192, 168, 1, 48};               // IP est�tico do arduino (est� configurado para o IP do grad 1)
//byte gateway[] = { x,x,x,x };                         // O gateway n�o precisou ser usado, por isso comentei
char dadosDoBanho[1000];                                // String onde escrevemos os dados (no formato JSON) para mandar pro servidor
char ipServidor[30] = "192.168.1.48";                    // IP local da m�quina onde o servidor est� rodando
char ipServidor1[30] = "192.168.1.48/ACESSO";                    // IP local da m�quina onde o servidor est� rodando
String mensagem;

EthernetClient client;

void setup()
{
  Ethernet.begin(mac);  // IP din�mico
//  Ethernet.begin(mac,ipArduino);  // IP est�tico
  Serial.begin(BAUD_RATE);
  Serial.print("Usando o seguinte IP: ");
  Serial.println(Ethernet.localIP());
  delay(2000);
  Serial.println("Conectando a rede local...");
}

void enviar(char *dados)
{
  if (client.connect(ipServidor,PORTA))
  {           
    Serial.println("Enviando POST ao servidor: ");
    client.println("POST / HTTP/1.1");          
    client.print("Host: ");
    client.println(ipServidor);
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println("User-Agent: Arduino/1.0");
    client.print("Content-Length: ");
    client.println(strlen(dados));
    client.println();
    client.print(dados);
    client.println();
    client.stop();
  }                               
  else
  {
    Serial.print("Nao foi possivel conectar ao servidor!");               
  }
}


void loop()                                           
{
  sprintf(dadosDoBanho, "{\"nome\" : \"fulano\", \"duracao\":%d}", 10);
  delay(ESPERA);
  enviar(dadosDoBanho);
}


