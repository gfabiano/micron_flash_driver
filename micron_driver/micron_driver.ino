// ( PINS

#define IO_START_ADDR 0

#define IO0 0
#define IO1 1
#define IO2 2
#define IO3 3
#define IO4 4


#define ALE 16
#define CE  17
#define CLE 18
#define RE  19
#define WE  20
#define READY 21

// PINS )

// ( COMMANDS
#define CMD_READ 0x30

// COMMANDS )


// setto i pin dell'arduino in input
void set_bus_read()
{
    for (size_t i = 0; i < 8; ++i){
        pinMode(i + IO_START_ADDR, INPUT); 
    }
}

// setto i pin dell'arduino in output
void set_bus_write()
{
    for (size_t i = 0; i < 8; ++i){
        pinMode(i + IO_START_ADDR, OUTPUT); 
    }
}

inline
void set_pin(uint8_t pin) { digitalWrite(pin, HIGH); }

inline
void unset_pin(uint8_t pin) {digitalWrite(pin, LOW); }

void setup() {
    set_bus_read();
    
    pinMode(ALE, OUTPUT); 
    pinMode(CE, OUTPUT);
    pinMode(CLE, OUTPUT);
    pinMode(RE, OUTPUT);
    pinMode(WE, OUTPUT);

    // attivi alti
    unset_pin(ALE);
    unset_pin(CE);
    unset_pin(CLE);


    // attivi bassi
    set_pin(RE);
    set_pin(WE);
    
    pinMode(READY, INPUT);
}

// dorme fino a quando la memoria non è pronta
// a ricevere un comando
void until_ready() {
     while (digitalRead(READY) == 0) {
        delay(10); 
    }
}


//CLE _________/----\___
//CE  ------\_______/---
//WE  ---\________/-----
//ALE ---------\____/---
//IO  xxxxxxxxx<cmd>xxxx
//
void send_command(uint8_t cmd)
{
    until_ready();
    set_bus_write();

    unset_pin(WE);
    unset_pin(CE);
    set_pin(CLE); // attivo alto
    unset_pin(ALE);

    // scrivo il comando sui pin
    for (size_t i = 0; i < 8; ++i) {
        uint8_t b = cmd & 0x01;
        digitalWrite(i + IO_START_ADDR, b);
        cmd >>= 1;
    }

    // scrivo sul fronte in salita di WE
    set_pin(WE);
}

void read_page()
{
    send_command(CMD_READ);

}


void loop() {


}
