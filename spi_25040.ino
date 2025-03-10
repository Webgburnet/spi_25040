// entrées / sortie
#define DATAOUT     11  //MOSI
#define DATAIN_     12  //MISO
#define SPICLOCK    13  //sck
#define SLAVESELECT 10  //ss

//opcodes
#define WREN  6
#define WRDI  4
#define RDSR  5
#define WRSR  1
#define READ  3
#define WRITE 2

byte buffer [16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};


byte spi_transfer(byte data)
{
  SPDR = data;                    // Start the transmission
  //while (!(SPSR & (1<<SPIF)));    // Wait the end of the transmission
  while ((SPSR & B10000000)==0);
  return SPDR;                    // return the received byte
}

void waitWhileWriting() {
  int data;
  do {
    delayMicroseconds(1);
    digitalWrite(SLAVESELECT,LOW);
    spi_transfer(RDSR); //transmit read opcode
    data=spi_transfer(0x00);
    //Serial.print("data -> ");Serial.println(data,DEC);
    digitalWrite(SLAVESELECT,HIGH);   
  } while ( data & 0x03 == 0x03); 
}

byte read_eeprom(word EEPROM_address) {
  byte data;
  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(READ | (byte)( (EEPROM_address & 0x100) >> 5));
  spi_transfer((byte)(EEPROM_address));    
  data = spi_transfer(0x00); 
  digitalWrite(SLAVESELECT,HIGH); 
  return data;
}

void read_eepromContinuously()
{
  byte data;
  word address=0;
  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(READ); 
  spi_transfer(address);    
  do {
    data = spi_transfer(0xFF);
    Serial.print(address,HEX);Serial.print("->");Serial.println(data,DEC);
    address++;
  }while(address<512);
  digitalWrite(SLAVESELECT,HIGH);
}

void read_eepromUntilEtoile()
{
  byte data;
  word address=0;
  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(READ); 
  spi_transfer(address);    
  do {
    data = spi_transfer(0x00);
    //Serial.print(address,HEX);Serial.print("->");Serial.println(data,DEC);
    Serial.print((char)data);
    address++;
  }while((address<512) and (data!='*'));
  digitalWrite(SLAVESELECT,HIGH);
  Serial.println();
}

void write_eeprom(word EEPROM_address, byte data) {

  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(WREN); 
  digitalWrite(SLAVESELECT,HIGH);
  delayMicroseconds(1);
  
  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(WRITE | (byte)( (EEPROM_address&0x100) >> 5) ); 
  EEPROM_address=(byte)(EEPROM_address&0x0FF);
  spi_transfer((char)(EEPROM_address));      
  spi_transfer(data); 
  digitalWrite(SLAVESELECT,HIGH);
  //attend la fin du cycle
  waitWhileWriting();

}

void ecrirePage(word address, byte buf[16]) {

  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(WREN); //write enable
  digitalWrite(SLAVESELECT,HIGH);
  delayMicroseconds(1);
  
  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(WRITE | (byte)( (address&0x100) >> 5) ); //write instruction
  address=(byte)(address&0x0FF);
  spi_transfer((char)(address));      
  //write 16 bytes
  for (int I=0;I<16;I++)
  {
    spi_transfer(buffer[I]); //write data byte
  }
  digitalWrite(SLAVESELECT,HIGH); //release chip
  //attend la fin du cycle
  waitWhileWriting();

}


void setup()
{
  Serial.begin(9600);

  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN_, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(SLAVESELECT,OUTPUT);
  digitalWrite(SLAVESELECT,HIGH); //disable device
  // SPCR = B01010001;
  SPCR = (1<<SPE)|(1<<MSTR);//|(1<<SPR1)|(1<<SPR0);
  byte data=SPSR;
  data=SPDR;
  delayMicroseconds(1);
}

void loop()
{
  /*byte data;
  write_eeprom(6, 60);
  data = read_eeprom(6);
  Serial.println(data);

  write_eeprom(0x110, 110);
  data = read_eeprom(0x110);
  Serial.println(data);

  ecrirePage(0x110, buffer);
  read_eepromContinuously();
  while(true);
  */

  read_eepromUntilEtoile();
  Serial.println("Entrez une chaine terminee par une etoile : ");
  word address=0;
  byte c=0;
  while(c!='*') {
    if (Serial.available()) {
      c=Serial.read();
      if (address==512) address=0;
      write_eeprom(address, c);
      address++; 
    }
  }
}
