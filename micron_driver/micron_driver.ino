// ( PINS

#define IO_START_ADDR 0


#define ALE 16
#define CE  17
#define CLE 18
#define RE  19
#define WE  20
#define READY 21

// PINS )

// ( COMMANDS
#define CMD_READ 0x00

// COMMANDS )

enum bus_mode_t{READ, WRITE};
bus_mode_t bus_mode{READ};

// setto i pin dell'arduino in input
inline
void set_bus_read()
{
    if (bus_mode != READ) {
        for (size_t i = 0; i < 8; ++i){
            pinMode(i + IO_START_ADDR, INPUT); 
        }
        bus_mode = READ;
    }
}

// setto i pin dell'arduino in output
inline
void set_bus_write()
{
    if (bus_mode != WRITE) {
        for (size_t i = 0; i < 8; ++i){
            pinMode(i + IO_START_ADDR, OUTPUT); 
        }
        bus_mode = WRITE;
    }
}

inline
void set_pin(uint8_t pin) { digitalWrite(pin, HIGH); }

inline
void unset_pin(uint8_t pin) {digitalWrite(pin, LOW); }

void setup() {
    Serial.begin(9600);
    set_bus_write();
    
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
inline
void until_ready() {
     while (digitalRead(READY) == 0) {
        delay(10); 
    }
}

void bus_set(uint8_t val)
{
    set_bus_write();
    for (size_t i = 0; i < 8; ++i) {
        uint8_t b = val & 0x01;
        digitalWrite(i + IO_START_ADDR, b);
        val >>= 1;
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
    
    unset_pin(WE);
    unset_pin(CE);
    set_pin(CLE); // attivo alto
    unset_pin(ALE);

    // scrivo il comando sui pin
    bus_set(cmd);

    // scrivo sul fronte in salita di WE
    set_pin(WE);
}


//CLE xx\_______________
//CE  xx\_______________
//WE  ---\__/-\__/-\__/-
//ALE ____/-----------\_
//IO  xxx<c1>x<c2>x<r1>x...
//
void send_address(uint16_t col_addr, uint8_t page_addr, uint8_t block_addr)
{
    unset_pin(CLE);
    unset_pin(CE);
    unset_pin(ALE);

    // primo ciclo
    unset_pin(WE);
    bus_set(col_addr & 0xff00);
    set_pin(WE);

    // secondo ciclo
    unset_pin(WE);
    bus_set(col_addr >> 8);
    set_pin(WE);

    // terzo ciclo
    unset_pin(WE);
    bus_set(page_addr & 0xc0 + (block_addr & 0x03) << 6);
    set_pin(WE);

    // quarto ciclo
    unset_pin(WE);
    bus_set(block_addr >> 2);
    set_pin(WE);
}

void send_address_cycle(uint8_t addr) {
    unset_pin(CLE);
    unset_pin(CE);
    unset_pin(ALE);

    unset_pin(WE);
    bus_set(addr);
    set_pin(WE);
}

uint8_t read_bus()
{
    set_bus_read();
    unset_pin(RE);
    unset_pin(CLE);
    unset_pin(CE);
    unset_pin(ALE);
    
    set_pin(RE); 

    uint8_t out{0};
    for (size_t i = 0; i < 8; ++i) {
        uint8_t b = digitalRead(i + IO_START_ADDR);
        out |= (b << i);
    }
    return out;
}

uint8_t read_mem_byte(uint16_t col_addr, uint8_t page_addr, uint8_t block_addr)
{
    send_address(col_addr, page_addr, block_addr);
    
    return read_bus();
}

void read_page()
{
    send_command(CMD_READ);
    for (size_t i = 0; i < 64; ++i) {
        uint8_t b = read_mem_byte(0, 0, 0);
        Serial.print(b, HEX);
    }
}

void read_id()
{
    send_command(0x90);
    send_address_cycle(0x00);

    
    uint8_t manufacturer = read_bus();
    uint8_t device_id = read_bus();

    Serial.print("manufacturer: ");
    Serial.print(manufacturer, HEX);
    Serial.print("device id: ");
    Serial.println(device_id, HEX);
}


void loop() {
    read_id();
    //read_page();


    // halt!
    while(true) {
        delay(50);
    }
}
