/**
 * @file main.c
 *
 * @brief Main function
 */

#include "mcu.h"
#include "serial.h"
#include "app_fatfs_spi.h"
#include "spi.h"
#include "gpio.h"

#include <string.h>

/*****************************************
 * Private Constant Definitions
 *****************************************/

#define LED_TOGGLE_DELAY_MS 1500

/*****************************************
 * Main Function
 *****************************************/

int main(void) {
    mcu_init();

    serial_init();
    MX_SPI1_Init();
    APP_FATFS_SPI_Init(&hspi1, GPIOA, GPIO_PIN_10);

    // Fatfs object
    FATFS FatFs;

    // File object
    FIL fil;

    printf("\r\n\r\n(Powering up)\r\nFATFS TEST --\r\nTEST STARTING\r\n\r\n");
    HAL_Delay(1000);
    FRESULT fres;

    int i = 0;

    for (;;) {
        led_toggle();

        for (; i < 1; i++) {
            fres = f_mount(&FatFs, "", 1);  // 1=mount now

            if (fres != FR_OK) {
                printf("f_mount error (%i)\r\n", fres);

                while (1)
                    ;
            }

            DWORD free_clusters, free_sectors, total_sectors;

            FATFS* getFreeFs;

            fres = f_getfree("", &free_clusters, &getFreeFs);

            if (fres != FR_OK) {
                printf("f_getfree error (%i)\r\n", fres);

                while (1)
                    ;
            }

            total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
            free_sectors = free_clusters * getFreeFs->csize;

            printf("SD card stats:\r\n%10lu KiB total drive space.\r\n%10lu KiB available.\r\n", total_sectors / 2,
                   free_sectors / 2);

            // Try to open file
            fres = f_open(&fil, "test.txt", FA_READ);

            if (fres != FR_OK) {
                printf("f_open error (%i)\r\n", fres);

                while (1)
                    ;
            }

            printf("I was able to open 'test.txt' for reading!\r\n");

            BYTE readBuf[30];

            // We can either use f_read OR f_gets to get data out of files
            // f_gets is a wrapper on f_read that does some string formatting for us
            TCHAR* rres = f_gets((TCHAR*) readBuf, 30, &fil);

            if (rres != 0) {
                printf("Read string from 'test.txt' contents: %s\r\n", readBuf);
            } else {
                printf("f_gets error (%i)\r\n", fres);
            }

            // Close file, don't forget this!
            f_close(&fil);

            fres = f_open(&fil, "write.txt", FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);

            if (fres == FR_OK) {
                printf("I was able to open 'write.txt' for writing\r\n");
            } else {
                printf("f_open error (%i)\r\n", fres);
            }

            strncpy((char*) readBuf, "a new file is made!", 19);
            UINT bytesWrote;
            fres = f_write(&fil, readBuf, 19, &bytesWrote);

            if (fres == FR_OK) {
                printf("Wrote %i bytes to 'write.txt'!\r\n", bytesWrote);
            } else {
                printf("f_write error (%i)\r\n", fres);
            }

            // Close file, don't forget this!
            f_close(&fil);

            // De-mount drive
            f_mount(NULL, "", 0);
        }
    }
}
