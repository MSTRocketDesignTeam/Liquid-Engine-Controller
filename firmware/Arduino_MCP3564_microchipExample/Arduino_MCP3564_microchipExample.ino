/*******************************************************************************
* Company:
*   Microchip Technology Inc.
*
* File Name:
*   Arduino_MCP3564_vX(API).cpp
*
* Summary:
*   This file contains the source code for the MCP3x6x(R) Delta-Sigma ADC 
*   Main Demo Code Loop function. 
*
* Description:
*   This file contains the source code for the MCP3x6x(R) Delta-Sigma ADC Main 
*   Demo Code Loop for use the Arduino Zero Board and the ATSAMD21G18 MCU.  
*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
/*                                                  Section: Included Files                                                                                */
/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include "MCP3x6x_Type_Definitions.h"                                                           // Variable definitions header-file.
#include "MCP3x6x_ADC_Definitions.h"                                                            // ADC definitions header-file.
#include "MCP3x6x_Peripheral_Definitions.h"                                                     // Peripheral definitions header-file.
#include "MCP3x6x_SPI_Definitions.h"                                                            // SPI definitions header-file.

/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
/*                                                  Section: Arduino Setup and Main Loop Functions                                                         */
/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/

void Microchip_Banner(void);                                                                    // Terminal "Microchip Technology Inc. Arduino Zero MCP3564 Code Example" Banner.
void Operation_Option_Menu(void);                                                               // Terminal Operations Main Menu options. 
void Register_Read_Option_Menu(void);                                                           // Terminal Register-Read Menu options. 
void Display_Output_Data(_u32data_t, _u32data_t, _u16data_t, uint64_t);                         // Terminal Data-Output Display function.  
void Cursor_Ln7_Col0(void);                                                                     // Terminal formatting function; Line 7 Column 0.
void Cursor_Ln9_Col0(void);                                                                     // Terminal formatting function; Line 9 Column 0.
void Cursor_Ln7_Col45(void);                                                                    // Terminal formatting function; Line 7 Column 45.
void Cursor_Ln9_Col45(void);                                                                    // Terminal formatting function; Line 9 Column 45.

/***********************************************************************************************************************************************************
  Function:
    void setup()

  Remarks:
    - Arduino, ADC, and Serial Port Configuration Functions. 
 */

void setup() 
{
    // put your setup code here, to run once:

    //ATSAMD21G18 SPI Configuration for Arduino Zero Board
    pinMode(PIN_SPI_nCS, OUTPUT);                                                               // nCS Pin is an output. See "MCP3x6x_SPI_Definitions.h."
                                                                                                // SCK, SDO and SDI Pins are configured explicitely for the pi-pico to work (no defaults)
    arduino::MbedSPI SPI(4, 7, 6); // GPIO4: MISO, GPIO7: MOSI, GPIO5: CS                                                            
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));                           // SPI Bus is 10MHz, MSb-First, Mode 0,0.   
    SPI.begin();                                                                                // SPI SCK and SDO Pins are Outputs.
                                                                                                // SPI SDI Pin is an Input.
    //MCP3564 Delta-Sigma Configuration
    MCP3x6x_CONFIG();                                                                           // MCP3564 Register Configuration.

    //ATSAMD21G18 Serial Port Configuration
    Serial.begin(115200);                                                                       // Serial terminal initialized for 115200 Baud.
   
    while(!Serial)                                                                              // Serial Port available? 
    {
      
      Microchip_Banner();                                                                     // Display "Microchip Technology Inc." Banner. 
    }    
}

/***********************************************************************************************************************************************************
  Function:
    void loop()

  Remarks:
    - Arduino Main-Loop Function. 
 */

void loop() 
{
    // put your main code here, to run repeatedly:
    _u32data_t CONV_DATA; 
    CONV_DATA.DWORD = 0x00000000;

    _u32data_t TEMP_SNSR_DATA;
    TEMP_SNSR_DATA.DWORD = 0x00000000;

    _u16data_t ADC_CRC;
    ADC_CRC.WORD = 0x0000;

    uint64_t CALC_CRC = 0x000000000000;
    
    while(1)
    {
        switch (Serial.read())                                                                  // Read terminal input value. 
        {
            case 0x31:                                                                          // "Signal Conversion" selected. 
                if (Serial)                                                                     // Is serial port available???
                {
                    Cursor_Ln7_Col0();                                                          // Set terminal cursor to Line 7 Column 0.
                    Serial.print(" Pending Signal Conversion... ");                             // Print " Pending Conversion... ".            
                    
                    delay(1500);                                                                // Delay for 1.5s.

                    Cursor_Ln7_Col0();                                                          // Set terminal cursor to Line 7 Column 0.                                                                                                  

                    if(CONV_START(MUX_VINN_CH0 | MUX_VINN_CH1) == 0x13)                         // Convert CH0(+) and CH1(-) channel and check Data-Ready(DR) Bit of STATUS Byte.               
                    {  
                        CONV_DATA.DWORD = SPI_RD(_ADCDATA_, ADC_CRC, CALC_CRC);                 // Read Signal Conversion data.   
                        Display_Output_Data(CONV_DATA, TEMP_SNSR_DATA, ADC_CRC, CALC_CRC);      // Print Signal Conversion, Temperature Sensor and Communication-CRC data.  
                    }
                    else                                                                        // No new Data-Ready? 
                    {
                        Serial.print(" Invalid Measurement!!! Displaying previous data. ");     // Print " Invalid Measurement!!! Displaying previous data. ".                                                          
                    }

                    delay(3000);                                                                // Delay for 3s.     
                }
            break; 
            
            case 0x32:                                                                          // "Temperature Measurement" selected.  
                if(Serial)                                                                      // Is serial port available???
                {
                    Cursor_Ln7_Col0();                                                          // Set terminal cursor to Line 7 Column 0
                    Serial.print(" Pending Temperature Measurement... ");                       // Print " Pending Temperature Measurement... ".
                    
                    delay(1500);                                                                // Delay for 1.5s.
                    
                    Cursor_Ln7_Col0();                                                          // Set terminal cursor to Line 7 Column 0. 
              
                    if(CONV_START(MUX_VINP_TMPP |  MUX_VINN_TMPM) == 0x13)                      // Convert Temperature Sensor channel and check Data-Ready(DR) Bit of STATUS Byte.
                    {                  
                        TEMP_SNSR_DATA.DWORD = SPI_RD(_ADCDATA_, ADC_CRC, CALC_CRC);            // Read Temperature Sensor data.  
                        CONV_DATA.DWORD = TEMP_SNSR_DATA.DWORD;                                 // Transfer TEMP_SNSR_DATA Structure data to CONV_DATA Structure for preservation.
                        TEMP_SNSR_DATA.FLTWORD = TEMP_SNSR(TEMP_SNSR_DATA.DWORD);               // Convert Temperature Sensor output data to °C. 
                        Display_Output_Data(CONV_DATA, TEMP_SNSR_DATA, ADC_CRC, CALC_CRC);      // Print Signal Conversion, Temperature Sensor and Communication-CRC data.
                    }
                    else                                                                        // No new Data-Ready? 
                    {                     
                        Serial.print(" Invalid Measurement!!! Displaying previous data. ");     // Print " Invalid Measurement!!! Displaying previous data. ".
                    }

                    delay(3000);                                                                // Delay for 3s.
                }  
            break;
            
            case 0x33:                                                                          // "Register Configuration" selected.  
                if(Serial)                                                                      // Is serial port available???
                {
                    uint8_t REG_ADDR;                                                           // Address of register Read-CMD. 
                    uint32_t REG_DATA;                                                          // REG_ADDR Register read-data variable. 
                    
                    Cursor_Ln7_Col0();                                                          // Set terminal cursor to Line 7 Column 0.
                    
                    delay(500);                                                                 // Delay for 500ms. 
                    
                    Register_Read_Option_Menu();                                                // Print Register Read-CMD menu optoins.
                    while(Serial.available() == 0){};                                           // Wait for key press. 
                    REG_ADDR = Serial.read();                                                   // Read key press from serial port. 

                    if((REG_ADDR == 0x31) | (REG_ADDR == 0x32) | (REG_ADDR == 0x33) | \        
                      (REG_ADDR == 0x34) | (REG_ADDR == 0x35) | (REG_ADDR == 0x36) | \ 
                      (REG_ADDR == 0x37) | (REG_ADDR == 0x38) | (REG_ADDR == 0x39))             // Number 1 to 9 pressed?     
                    {
                        REG_DATA = SPI_RD((REG_ADDR - 0x30), ADC_CRC, CALC_CRC);
                    }
                    else if ((REG_ADDR == 0x41) | (REG_ADDR == 0x44) | (REG_ADDR == 0x46))      // Capital letters pressed? 
                    {
                        REG_DATA = SPI_RD((REG_ADDR - 0x37), ADC_CRC, CALC_CRC);
                    }
                    else if ((REG_ADDR == 0x61) | (REG_ADDR == 0x64) | (REG_ADDR == 0x66))      // Lower-case letters pressed? 
                    {
                        REG_DATA = SPI_RD((REG_ADDR - 0x57), ADC_CRC, CALC_CRC);
                    }
                    else                                                                        // Invalid key press? 
                    {
                        Serial.print(" Invalid Register Requested!!! ");                        // Print " Invalid Register Requested!!! ". 
                        
                        delay(3000);                                                            // Delay for 3s.
                        break;
                    }

                    delay(500);                                                                 // Delay for 500ms.
                    
                    Serial.print(" Register Data = 0x");                                        // Print " Register Data = 0x". 
                    Serial.print((REG_DATA & 0x00FFFFFF), HEX);                                 // Print REG_ADDR Register read-data. 
                    
                    delay(3000);                                                                // Delay for 3s.
                }  
            break;
            
            default:              
                if (Serial)                                                                     // Is serial port available???
                {
                    Microchip_Banner();                                                         // Print "Microchip Technology Inc." Banner
                    Display_Output_Data(CONV_DATA, TEMP_SNSR_DATA, ADC_CRC, CALC_CRC);          // Print Signal Conversion, Temperature Sensor and Communication-CRC data. 
                    Operation_Option_Menu();                                                    // Print Main Operation menu options. 
                    
                    delay(2000);                                                                // Delay for 2s.                                                            
                }
            break;
        }
    }
}

/***********************************************************************************************************************************************************
  Function:
    void Microchip_Banner(void)

  Remarks:
    - Microchip Terminal Banner Function. 
 */

void Microchip_Banner(void)                                                                     // Print "Microchip Technology Inc." banner.
{
    Serial.print("\x1B[20h");                                                                   // Set serial port to "New Line Mode."
    Serial.print("\x1B[f");                                                                     // Set cursor to origin (RealTerm Serial Capture). 
    Serial.print("\x1B[2J");                                                                    // Clear entire screen (RealTerm Serial Capture).
              
    Serial.print(" *******************************************************************************\n");
    Serial.print(" *                           Microchip Technology Inc.                         *\n");
    Serial.print(" *                           MCP3564 Arduino Zero                              *\n");
    Serial.print(" *                           Code Example                                      *\n");
    Serial.print(" *******************************************************************************\n");   
}

/***********************************************************************************************************************************************************
  Function:
    void Operation_Option_Menu(void)

  Remarks:
    - Main Operation Menu Function. 
 */

void Operation_Option_Menu(void)                                                                // Print Main Operation menu options. 
{
    Serial.print("\n\n ******************************************************************************");

    Serial.print("\n\n Select Option: ");                                                       // Print " Select Option: ". 
    Serial.print("\n\n\t1.) Signal Conversion? ");                                              // Print " Signal Conversion? ".
    Serial.print("\n\n\t2.) Temperature Measurement? ");                                        // Print " Temperature Measurement? ".
    Serial.print("\n\n\t3.) Register Configuration? ");                                         // Print " Register Configuration? ".
}

/***********************************************************************************************************************************************************
  Function:
    void Register_Read_Option_Menu(void)

  Remarks:
    - Register Read-CMD Options Menu Function. 
 */

void Register_Read_Option_Menu(void)                                                            // Print Register Read-CMD menu optoins. 
{
    Serial.print(" ****************** ");                                                        
    Serial.print("\n *  Register Map  * ");                                                     // Print " Register Map ".
    Serial.print("\n ****************** ");
    Serial.print("\n 1.) CONFIG0 ");                                                            // Print " 1.) CONFIG0 ".
    Serial.print("\n 2.) CONFIG1 ");                                                            // Print " 2.) CONFIG1 ".
    Serial.print("\n 3.) CONFIG2 ");                                                            // Print " 3.) CONFIG2 ".
    Serial.print("\n 4.) CONFIG3 ");                                                            // Print " 4.) CONFIG3 ".
    Serial.print("\n 5.) IRQ ");                                                                // Print " 5.) IRQ ".
    Serial.print("\n 6.) MUX ");                                                                // Print " 6.) MUX ".
    Serial.print("\n 7.) SCAN ");                                                               // Print " 7.) SCAN ".
    Serial.print("\n 8.) TIMER ");                                                              // Print " 8.) TIMER ".
    Serial.print("\n 9.) OFFSETCAL ");                                                          // Print " 9.) OFFSETCAL ".
    Serial.print("\n A.) GAINCAL ");                                                            // Print " A.) GAINCAL ".
    Serial.print("\n D.) LOCK ");                                                               // Print " D.) LOCK ".
    Serial.print("\n F.) CRCCFG ");                                                             // Print " F.) CRCCFG ".

    Serial.print("\n\n Select Register < 1 to F >: ");                                          // Print " Select Register < 1 to F >: ".
}

/***********************************************************************************************************************************************************
  Function:
    void Display_Output_Data(_u32data_t, _u32data_t, _u16data_t, uint64_t)

  Remarks:
    - Signal Conversion, Temperature Sensor, Communication-CRC Data and Calculated-CRC Data Terminal Output Function. 
 */

void Display_Output_Data(_u32data_t CONV_DATA, _u32data_t TEMP_SNSR_DATA, _u16data_t ADC_CRC, uint64_t CALC_CRC)          // Print Signal Conversion, Temperature Sensor, Communication-CRC data, and Calculated-CRC data.  
{ 
    Cursor_Ln7_Col0();
    Serial.print(" Conversion Data = 0x");                                                      // Print " Conversion Data = 0x".
    Serial.print((CONV_DATA.DWORD & 0x00FFFFFF), HEX);                                          // Print CONV_DATA variable in hex format. 
    Serial.print(" (");
    Serial.print((CONV_DATA.DWORD & 0x00FFFFFF), DEC);                                          // Print CONV_DATA variable in decimal format.
    Serial.print(" Dec)");

    Cursor_Ln7_Col45();
    Serial.print(" Current Temperature = ");                                                    // Print " Current Temperature = "
    Serial.print(TEMP_SNSR_DATA.FLTWORD);                                                       // Print TEMP_SNSR_OUTPUT variable in floating point format.                                                
    Serial.print(char(176));                                                                    // Print degree symbol. 
    Serial.print("C");                                                                          // Print "C" for Celcius.                           
    /* (NOTE: Tera-Term Terminal program must have font set to anything other than "Terminal" to properly display degree symbol. */

    Cursor_Ln9_Col0();
    Serial.print(" Communication-CRC = 0x");                                                    // Print " Communication-CRC = 0x". 
    Serial.print(ADC_CRC.WORD, HEX);                                                            // Print COMM_CRC variable in hex format. 
              
    Cursor_Ln9_Col45();
    Serial.print(" Calculated-CRC = 0x");                                                       // Print " Calculated-CRC = 0x".
    Serial.print(((uint16_t)(CALC_CRC >> 16)), HEX);                                            // Print CRC_DIVD_64b in hex format. 
}

/***********************************************************************************************************************************************************
  Function:
    void Cursor_LnX_ColY(void)

  Remarks:
    - Set Terminal Cursor at Line X and Column Y Functions. 
 */

void Cursor_Ln7_Col0(void)
{
    Serial.print("\x1B[7;0H");                                                                  // Set terminal cursor to Line 7, Column 0 (RealTerm Serial Capture).
    Serial.print("\x1B[J");                                                                     // Erase from terminal cursor position to end of screen (RealTerm Serial Capture).
}

void Cursor_Ln9_Col0(void)
{
    Serial.print("\x1B[9;0H");                                                                  // Set terminal cursor to Line 9, Column 0 (RealTerm Serial Capture).
    Serial.print("\x1B[J");                                                                     // Erase from terminal cursor position to end of screen (RealTerm Serial Capture).
}

void Cursor_Ln7_Col45(void)
{
    Serial.print("\x1B[7;45H");                                                                 // Set terminal cursor to Line 7, Column 45 (RealTerm Serial Capture).
    Serial.print("\x1B[J");                                                                     // Erase from terminal cursor position to end of screen (RealTerm Serial Capture).
}

void Cursor_Ln9_Col45(void)
{
    Serial.print("\x1B[9;45H");                                                                 // Set termial cursor to Line 9, Column 45 (RealTerm Serial Capture).
    Serial.print("\x1B[J");                                                                     // Erase from terminal cursor position to end of screen (RealTerm Serial Capture).
}

/**********************************************************************************************************************************************************
 End of File
 */
