#include "bme_userspace.h"

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#include "bme280.h"
#include "lcd.h"

struct identifier
{
    uint8_t dev_addr;
    int8_t fd;
};

int8_t user_i2c_read(uint8_t reg_addr, uint8_t* data, uint32_t len, void* intf_ptr) {
    struct identifier id;

    id = *((struct identifier*)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

void user_delay_us(uint32_t period, void* intf_ptr) {
    usleep(period * 1000);
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t* data, uint32_t len, void* intf_ptr) {
    uint8_t* buf;
    struct identifier id;

    id = *((struct identifier*)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(id.fd, buf, len + 1) < (uint16_t)len) {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}

double get_temperatura() {

    struct bme280_dev dev;
    struct identifier id;
    uint32_t req_delay;
    int8_t rslt = BME280_OK;

    if ((id.fd = open("/dev/i2c-1", O_RDWR)) < 0) {
        exit(1);
    }

    id.dev_addr = BME280_I2C_ADDR_PRIM;

    if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0) {
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    dev.intf_ptr = &id;

    rslt = bme280_init(&dev);
    if (rslt != BME280_OK) {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
        exit(1);
    }

    if (rslt != BME280_OK) {
        fprintf(stderr, "Failed to stream sensor data (code %+d).\n", rslt);
        exit(1);
    }

    rslt = BME280_OK;

    uint8_t settings_sel = 0;
    struct bme280_data comp_data;

    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    rslt = bme280_set_sensor_settings(settings_sel, &dev);

    req_delay = bme280_cal_meas_delay(&dev.settings);
    dev.delay_us(req_delay, dev.intf_ptr);
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);

    if (rslt != BME280_OK) {
        fprintf(stderr, "Failed to get sensor data (code %+d).", rslt);
    }

    return comp_data.temperature;

}