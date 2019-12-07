// -*- c++ -*-

#define SHIFT_DATA       2
#define SHIFT_CLOCK      3
#define SHIFT_LATCH      4
#define EEPROM_D0        5
#define EEPROM_D7        12

#define EEPROM_CHIP_EN   14
#define EEPROM_OUT_EN    15
#define EEPROM_WR_EN     16

#define EEPROM_WRITE_TIMEOUT_MS 15    // timeout for EEPROM write (ms)
#define SERIAL_READ_TIMEOUT_MS  5000  // timeout for serial read of write buffer (ms)

unsigned char write_buf[512];

char tmp_buf[80];
unsigned int tmp_buf_size;

char cmd_buf[32];
unsigned int cmd_buf_size;
unsigned int cmd_buf_pos;
bool cmd_error_flag;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  
  eeprom_init();
  
  Serial.begin(115200);
  Serial.println("\n*READY");
}

void loop()
{
  process_serial();
}

// ==============================================================================
// === Serial interface =========================================================
// ==============================================================================

void process_serial()
{
  while (Serial.available() > 0) {
    int b = Serial.read();
    if (b < 0) break;
    cmd_buf[cmd_buf_size++] = b;

    if (cmd_buf_size > 0 && cmd_buf[cmd_buf_size-1] == '\n') {
      cmd_buf_size--;
      process_cmd();
      cmd_buf_size = 0;
    } else if (cmd_buf_size >= (unsigned int) sizeof(cmd_buf)) {
      process_cmd();
      cmd_buf_size = 0;
    }
  }
}

void process_cmd()
{
  if (cmd_buf_size == 0) {
    return;
  }

  cmd_error_flag = false;
  cmd_buf_pos = 1;
  switch (cmd_buf[0]) {
  case 'r': process_read_cmd();    break;
  case 'w': process_write_cmd();   break;
  case 'd': process_dump_cmd();    break;
  default:  process_invalid_cmd(); break;
  }

  if (! cmd_error_flag) {
    Serial.println("\n*READY");
  }
}

void cmd_error(const char *msg)
{
  Serial.print("*ERROR: ");
  Serial.println(msg);
  cmd_error_flag = true;
}

int hex_to_nibble(char ch)
{
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  return -1;
}

char nibble_to_hex(unsigned int nibble)
{
  if (nibble < 10) {
    return nibble + '0';
  }
  return nibble - 10 + 'a';
}

void write_char(char c)
{
  tmp_buf[tmp_buf_size++] = c;
}

void write_hex_u8(byte b)
{
  tmp_buf[tmp_buf_size++] = nibble_to_hex(b >> 4);
  tmp_buf[tmp_buf_size++] = nibble_to_hex(b & 0x0f);
}

void write_hex_u16(unsigned int b)
{
  tmp_buf[tmp_buf_size++] = nibble_to_hex((b >> 12) & 0x0f);
  tmp_buf[tmp_buf_size++] = nibble_to_hex((b >>  8) & 0x0f);
  tmp_buf[tmp_buf_size++] = nibble_to_hex((b >>  4) & 0x0f);
  tmp_buf[tmp_buf_size++] = nibble_to_hex((b >>  0) & 0x0f);
}

unsigned int read_hex()
{
  while (cmd_buf_pos < cmd_buf_size && cmd_buf[cmd_buf_pos] == ' ') {
    cmd_buf_pos++;
  }
  unsigned int start_pos = cmd_buf_pos;

  unsigned int num = 0;
  while (cmd_buf_pos < cmd_buf_size) {
    int nibble = hex_to_nibble(cmd_buf[cmd_buf_pos]);
    if (nibble < 0) {
      break;
    }
    num = (num << 4) | nibble;
    cmd_buf_pos++;
  }

  if (start_pos == cmd_buf_pos) {
    cmd_error("expected number");
  }
  return num;
}

// --- INVALID
void process_invalid_cmd()
{
  tmp_buf_size = 0;
  write_hex_u8(cmd_buf[0]);
  Serial.print("invalid command byte: ");
  Serial.println(tmp_buf);
  cmd_error("invalid command");
}

// --- READ
void process_read_cmd()
{
  unsigned int addr = read_hex();
  if (cmd_error_flag) return;
  unsigned int len = read_hex();
  if (cmd_error_flag) return;

  tmp_buf_size = 0;
  while (len-- > 0) {
    if (tmp_buf_size+2 >= (unsigned int) sizeof(tmp_buf)) {
      write_char('\0');
      Serial.println(tmp_buf);
      tmp_buf_size = 0;
    }
    byte b = read_eeprom_byte(addr++);
    write_hex_u8(b);
  }
  if (tmp_buf_size > 0) {
    write_char('\0');
    Serial.println(tmp_buf);
  }
}

// --- DUMP
void process_dump_cmd()
{
  unsigned int addr = read_hex();
  if (cmd_error_flag) return;
  unsigned int len = read_hex();
  if (cmd_error_flag) return;

  unsigned int end = addr + len;
  while (addr != end) {
    unsigned int line_len = end - addr;
    if (line_len > 16) line_len = 16;
    for (unsigned int i = 0; i < line_len; i++) {
      cmd_buf[i] = read_eeprom_byte(addr+i);
    }
    dump_line(cmd_buf, line_len, addr);
    addr += line_len;
  }
}

void dump_line(char *d, unsigned int len, unsigned int addr)
{
  tmp_buf_size = 0;
  
  write_hex_u16(addr);
  write_char(':');
  write_char(' ');
  for (unsigned int i = 0; i < len; i++) {
    write_char(' ');
    write_hex_u8(d[i]);
  }
  for (unsigned int i = len; i < 16; i++) {
    write_char(' ');
    write_char(' ');
    write_char(' ');
  }
  write_char(' ');
  write_char(' ');
  for (unsigned int i = 0; i < len; i++) {
    write_char((d[i] >= 0x20 && d[i] < 0x7f) ? d[i] : '.');
  }
  write_char('\0');
  Serial.println(tmp_buf);
}

// --- WRITE
void process_write_cmd()
{
  unsigned int addr = read_hex();
  if (cmd_error_flag) return;
  unsigned int len = read_hex();
  if (cmd_error_flag) return;

  if (len == 0 || len > (unsigned int) sizeof(write_buf)) {
    cmd_error("invalid length");
    Serial.print("len: ");
    Serial.println(len);
    return;
  }
  
  // read data from serial
  unsigned int write_buf_len = 0;
  unsigned long start_time = millis();
  unsigned long timeout_time = start_time + SERIAL_READ_TIMEOUT_MS;
  int high_nibble = -1;
  while (write_buf_len < len) {
    if (millis() > timeout_time) {
      cmd_error("read timeout");
      return;
    }

    while (Serial.available() > 0 && write_buf_len < len) {
      int c = Serial.read();
      if (c >= 0) {
        int nibble = hex_to_nibble(c);
        if (nibble < 0) {
          cmd_error("invalid data");
          return;
        }
        if (high_nibble < 0) {
          high_nibble = nibble << 4;
        } else {
          write_buf[write_buf_len++] = high_nibble | nibble;
          high_nibble = -1;
        }
      }
    }
    if (write_buf_len < len) {
      delay(1);
    }
  }

  // write data to EEPROM
  digitalWrite(LED_BUILTIN, HIGH);
  for (unsigned int i = 0; i < write_buf_len; i++) {
    write_eeprom_byte(addr + i, write_buf[i]);
  }
  digitalWrite(LED_BUILTIN, LOW);

  /*
  Serial.println("dump buffer:");
  for (unsigned int dump_pos = 0; dump_pos < write_buf_len; dump_pos += 16) {
    unsigned int line_len = write_buf_len - dump_pos;
    if (line_len > 16) line_len = 16;
    dump_line((char *)write_buf + dump_pos, line_len, addr + dump_pos);
  }
  */
}

// ==============================================================================
// === EEPROM interface =========================================================
// ==============================================================================

void eeprom_init()
{
  pinMode(SHIFT_DATA,  OUTPUT);
  pinMode(SHIFT_CLOCK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(EEPROM_WR_EN,  HIGH);
  digitalWrite(EEPROM_OUT_EN, HIGH);
  pinMode(EEPROM_OUT_EN,      OUTPUT);
  pinMode(EEPROM_WR_EN,       OUTPUT);
  pinMode(EEPROM_CHIP_EN,     OUTPUT);
}

void set_eeprom_address(unsigned int address)
{
  // shift data intp shift registers
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, (address >> 8) & 0xff);
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, (address >> 0) & 0xff);

  // latch value into shift registers' output
  digitalWrite(SHIFT_LATCH, LOW);
  delayMicroseconds(1);
  digitalWrite(SHIFT_LATCH, HIGH);
  delayMicroseconds(1);
  digitalWrite(SHIFT_LATCH, LOW);
}

byte read_eeprom_byte(unsigned int address)
{
  // set EEPROM address
  set_eeprom_address(address);

  // enable EEPROM data output
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    pinMode(pin, INPUT);
  }
  digitalWrite(EEPROM_OUT_EN, LOW);
  
  // read EEPROM data
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    data = (data<<1) | (digitalRead(pin) ? 1 : 0);
  }
  return data;
}

void write_eeprom_byte(unsigned int address, byte data)
{
  // disable EEPROM data output
  digitalWrite(EEPROM_OUT_EN, HIGH);

  // setup EEPROM address and data
  set_eeprom_address(address);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    pinMode(pin, OUTPUT);
  }
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    digitalWrite(pin, data & 1);
    data >>= 1;
  }

  // pulse EEPROM write enable pin
  noInterrupts();
  digitalWrite(EEPROM_WR_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(EEPROM_WR_EN, HIGH);
  interrupts();

  // enable EEPROM data output
  digitalWrite(EEPROM_OUT_EN, LOW);
  
  // wait for EEPROM data to match written data
  byte got_data = 0;
  long start_time = millis();
  long cur_time = start_time;
  long max_time = cur_time + EEPROM_WRITE_TIMEOUT_MS;
  do {
    delay(1);
    for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
      got_data = (got_data<<1) | (digitalRead(pin) ? 1 : 0);
    }
    cur_time = millis();
  } while (got_data != data && cur_time < max_time);

  // disable EEPROM output
  digitalWrite(EEPROM_OUT_EN, HIGH);

  //Serial.print("waited for ");
  //Serial.print(cur_time - max_time);
  //Serial.println("ms");
}
