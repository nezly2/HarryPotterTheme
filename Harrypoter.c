#include <stdio.h>
#include "board.h"
#include "peripherals.h" 
#include "pin_mux.h"
#include "clock_config.h" 
#include "MKL25Z4.h"
#include "fsl_debug_console.h" 
#include "fsl_adc16.h" 
#include "fsl_tpm.h"
#include "config.h" 
#include "comp.h" 

#define c_nota 65 
#define Filtro 250 
int filtro(int pot);
int arreglo[Filtro];
int notas[c_nota] = {B_3,E_4,G_4,F_4,E_4,B_4,A_4,G_4,E_4,G_4,F_4,D_4,F_4,B_3,B_3,0,B_3,E_4,G_4,F_4,E_4,B_4,D_5,B_4b,C_5,G_4b,C_5,B_4,A_4b,A_3b,G_4,E_4,
E_4,G_4,B_4,G_4,B_4,G_4, C_5,B_4,A_4b,F_4,G_4,B_4,A_4b,A_3b,B_3,B_4,B_4,G_4,B_4,G_4,B_4,G_4,D_5,B_4b,C_5,G_4b,C_5,B_4,A_4b,A_3b,G_4,E_4,E_5};

int tiempo[c_nota] = {u_cu,t_oc,u_oc,u_cu,u_me,u_cu,t_cu,t_cu,t_oc,u_oc,u_cu,u_me,u_cu,t_cu,u_me,sil_un,u_cu,t_oc,u_oc,u_cu,u_me,u_cu,u_me,u_cu,u_me,u_cu,
t_oc,u_oc,u_cu,u_me,u_cu,t_cu,u_me,u_cu,u_me, u_cu,u_me,u_cu,u_me,u_cu,u_me,u_cu,t_oc,u_oc,u_cu,u_me,u_cu,t_cu,u_me,u_cu,u_me,u_cu,u_me,u_cu,u_me,u_cu,
u_me,u_cu,t_oc,u_oc,u_cu,u_me,u_cu,t_cu,t_cu};

unsigned int count, var_b = 0, count_u;

void PORTD_IRQHandler(void){ 
  uint32_t boton = 0;
  boton = GPIO_GetPinsInterruptFlags(GPIOD); 
  if(boton == 0x1){
    var_b = 1;
  }else if(boton == 0x20){
    var_b = 2; 
  }
  GPIO_ClearPinsInterruptFlags(GPIOD, (1U << 0)); 
  GPIO_ClearPinsInterruptFlags(GPIOD, (1U << 5));
}

int main(void) {
  BOARD_InitBootPins(); 
  BOARD_InitBootClocks(); 
  BOARD_InitBootPeripherals();
  #ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL 
    BOARD_InitDebugConsole();
  #endif

  EnableIRQ(PORTD_IRQn); 
  config();
  int pot, var, variable, cont = 0, nota=0, time=0, i; 
  while(1){
    GPIO_SetPinsOutput(GPIOB, 0x100); 
    switch(var_b){
      case 1: 
            GPIO_ClearPinsOutput(GPIOB, 0x100); 
            for(i=0; i<c_nota; i++){
              time = tiempo[i];
              nota = notas[i]; count_u = 0;
              while( time >= count_u){
                if(count >= nota){ 
                  GPIO_TogglePinsOutput(GPIOE, 0x400000); 
                  count= 0;
                }
              }
            }
            var_b = 0;
            break;
            
       case 2: 
            while(1){
                var_b = 0; 
                GPIO_ClearPinsOutput(GPIOB, 0x100); 
                ADC16_SetChannelConfig(ADC0, 0, &adc16ChannelConfigStruct);
                while (0U == (kADC16_ChannelConversionDoneFlag & ADC16_GetChannelStatusFlags(ADC0, 0))){}
                pot = ADC16_GetChannelConversionValue(ADC0, 0);
                pot = filtro(pot);//FILTRADO 
                if(pot < 53000 && pot>0){
                    var = 20 + (pot * .01849056604);
                }
                if(pot < 65535 && pot>53000){
                     cont = pot - 53000;
                     var = 1000 + (cont * 1.914791766); 
                     cont = 0;     
                 }
                 variable = (100000/(2*var));
                 if(count >= variable){
                    GPIO_TogglePinsOutput(GPIOE, 0x400000); 
                    count= 0;
                 }
                 if(var_b == 2){ 
                      break;
                 }
            }
            var_b = 0;
            break; 
    }
  }
}

void TPM2_IRQHandler(void){
    TPM_ClearStatusFlags(TPM2, kTPM_TimeOverflowFlag); 
    count++;
}

void TPM1_IRQHandler(void){ 
    TPM_ClearStatusFlags(TPM1, kTPM_TimeOverflowFlag); 
    count_u++;
}

int filtro(int pot){
    int i, PotFilter; 
    for(i=Filtro-1; i>0; i--){
        arreglo[i] = arreglo[i-1]; 
     }
     arreglo[0] = pot; 
     PotFilter = 0;
     for(i=Filtro-1; i>=0; i--){
          PotFilter = PotFilter + arreglo[i]; 
     }
     pot = PotFilter / Filtro; 
     return pot;
}

Librería de notas y tiempos (comp.h)
#ifndef COMP_H_ #define COMP_H_
//NOTAS
#define B_3 202 
#define E_4 152 
#define G_4 128 
#define F_4 143 
#define B_4 101 
#define A_4 114 
#define D_4 170 
#define D_5 84 
#define B_4b 107 
#define sil_un 20 
#define C_5 96 
#define A_4b 120 
#define A_3b 241 
#define G_4b 135 
#define E_5 76
//TIEMPOS
#define t_oc 375 
#define u_oc 125 
#define u_cu 250 
#define u_me 500 
#define t_cu 750
#endif /* COMP_H_ */
 
 
Configuración timmers y ADC (config.c)
#include "fsl_adc16.h" 
#include "fsl_tpm.h"

void config(void){

//estructura inicial ADC
adc16_config_t adc16ConfigStruct;//configurar el ADC 
adc16_channel_config_t adc16ChannelConfigStruct;//Configurar 
ADC16_GetDefaultConfig(&adc16ConfigStruct);// 
adc16ConfigStruct.resolution = kADC16_ResolutionSE16Bit;// 
ADC16_Init(ADC0, &adc16ConfigStruct);//Inicia el ADC0 
ADC16_EnableHardwareTrigger(ADC0, false);// 
ADC16_DoAutoCalibration(ADC0);//Función para calibrar el ADC 
adc16ChannelConfigStruct.channelNumber = 0;//Para escoger el canal 
adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false; 
adc16ChannelConfigStruct.enableDifferentialConversion = false;

//estructura inicial Timer para Tiempos
tpm_config_t tpmInfo1; 
CLOCK_SetTpmClock(1U);
TPM_GetDefaultConfig(&tpmInfo1);
TPM_Init(TPM1, &tpmInfo1);
TPM_SetTimerPeriod(TPM1, 48000);
TPM_EnableInterrupts(TPM1, kTPM_TimeOverflowInterruptEnable); 
EnableIRQ(TPM1_IRQn);
TPM_StartTimer(TPM1, kTPM_SystemClock);

//estructura inicial Timer para Notas
tpm_config_t tpmInfo; 
CLOCK_SetTpmClock(1U);
TPM_GetDefaultConfig(&tpmInfo);
TPM_Init(TPM2, &tpmInfo);
TPM_SetTimerPeriod(TPM2, 480);
TPM_EnableInterrupts(TPM2, kTPM_TimeOverflowInterruptEnable); 
EnableIRQ(TPM2_IRQn);
TPM_StartTimer(TPM2, kTPM_SystemClock);

}
