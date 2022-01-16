#include "menu_console.h"

#include <stdio.h>
#include <unistd.h>
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

#include "esp_log.h"

#include "menu_entry.h"

MenuEntry*        MenuConsole::m_menu;
MenuEntry*        MenuConsole::m_currentMenu;
StaticSemaphore_t MenuConsole::m_varMutexBuf;
SemaphoreHandle_t MenuConsole::m_varMutex;


MenuConsole::MenuConsole(int port_num)
{
   m_portNum = port_num;
   /* Drain stdout before reconfiguring it */
   fflush(stdout);
   fsync(fileno(stdout));

   /* Disable buffering on stdin */
   setvbuf(stdin, NULL, _IONBF, 0);

   /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
   esp_vfs_dev_uart_port_set_rx_line_endings(m_portNum, ESP_LINE_ENDINGS_CR);
   /* Move the caret to the beginning of the next line on '\n' */
   esp_vfs_dev_uart_port_set_tx_line_endings(m_portNum, ESP_LINE_ENDINGS_CRLF);

   /* Configure UART. Note that REF_TICK is used so that the baud rate remains
   * correct while APB frequency is changing in light sleep mode.
   */
   const uart_config_t uart_config = {
         .baud_rate  = 115200,
         .data_bits  = UART_DATA_8_BITS,
         .parity     = UART_PARITY_DISABLE,
         .stop_bits  = UART_STOP_BITS_1,
         .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
         .source_clk = UART_SCLK_REF_TICK,
   };
   /* Install UART driver for interrupt-driven reads and writes */
   ESP_ERROR_CHECK( uart_driver_install(m_portNum,
         256, 0, 0, NULL, 0) );
   ESP_ERROR_CHECK( uart_param_config(m_portNum, &uart_config) );

   /* Tell VFS to use UART driver */
   esp_vfs_dev_uart_use_driver(m_portNum);

   /* Initialize the console */
   esp_console_config_t console_config = {
         .max_cmdline_length = 256,
         .max_cmdline_args = 8,
#if CONFIG_LOG_COLORS
         .hint_color = atoi(LOG_COLOR_CYAN)
#endif
   };
   ESP_ERROR_CHECK( esp_console_init(&console_config) );

   /* Configure linenoise line completion library */
   /* Enable multiline editing. If not set, long commands will scroll within
   * single line.
   */
   linenoiseSetMultiLine(1);
   linenoiseAllowEmpty(true);
   /* Tell linenoise where to get command completions and hints */
   linenoiseSetCompletionCallback(&esp_console_get_completion);
   linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);
   linenoiseHistorySetMaxLen(100);
#if CONFIG_STORE_HISTORY
   /* Load command history from filesystem */
   linenoiseHistoryLoad(HISTORY_PATH);
#endif
   m_varMutex = xSemaphoreCreateMutexStatic(&m_varMutexBuf);
}

