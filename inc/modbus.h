#ifndef _MODBUS_
#define _MODBUS_

int write_modbus(char subcodigo, void *dado);
int read_modbus(char subcodigo, void *dado);

#endif