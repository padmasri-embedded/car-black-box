 /*
 * File:   main.c
 * Author: padmasri
 *
 * Created on 19 March, 2025, 12:00 AM
 */


#include "main.h"
#pragma config WDTE =OFF
extern unsigned char return_time;
extern unsigned int change_pass;
extern unsigned int long_press;
extern unsigned int short_press;

void init_config()
{
    init_i2c(100000);//initialize i2c
    init_clcd();//initialize clcd
    init_adc();//initialize adc
    init_ds1307();//initialize rtc
    init_digital_keypad();//initialize dkp
    init_timer2();//initialize timer
    init_uart(9600);//initilaization of uart
    PEIE=1;//peripheral inerrupt enable bit for timer 2
    GIE=1;//enable global interrupt
}

void main(void)
{
    //declaring the variables
    unsigned char control_flag=DASHBOARD_SCREEN,key;
    unsigned char event[3] ="ON";
    unsigned char reset_flag;
    unsigned char speed=0;
    unsigned char *gear[]={"GN","GR","G1","G2","G3","G4"};//gears in array
    unsigned char gr=0;
    static unsigned char menu_pos=0;
    //char ch;
    init_config();
    log_event(event,speed);
    ext_eeprom_24C02_str_write(0x00,"1010");
    while(1)
    {
    speed=read_adc()/10.3;
    if(speed >99)
    {
        speed=99;
    }
    key=read_digital_keypad();
    if(key==SW1)
    {
        //collision
        strcpy(event,"CO");
        log_event(event,speed);
    }
    if(key==SW2&& gr<6)
    {
        strcpy(event,gear[gr]);
        gr++;
        log_event(event,speed);
    }
    if(key==SW3 && gr>0)
    {
        gr--;
        strcpy(event,gear[gr]);
        log_event(event,speed);
    }
    else if((key==SW4||key==SW5)&&control_flag==DASHBOARD_SCREEN)
    {
        control_flag=LOGIN_SCREEN;
        clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
        __delay_us(500);
        clcd_print("Enter password",LINE1(1));
        clcd_write(LINE2(4),INST_MODE);
        clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
        __delay_us(100);
        reset_flag=RESET_PASSWORD;
        TMR2ON =1;

    }
    else if (key == SW4_LP && control_flag == MAINMENU_SCREEN)
    {
            switch (menu_pos)
            {
                case 0: //View Log 

                    clear_screen();
                    clcd_print("      Logs      ", LINE1(0));
                    control_flag = VIEW_LOG;
                    reset_flag = RESET_VIEW_LOG_POS;
                    break;

                case 1: // Clear Log 

                    clear_screen();
                    control_flag = CLEAR_LOG;
                    reset_flag = RESET_MEMORY;
                    break;

                case 2: //Download Log 

                    clear_screen();
                    log_event("DL", speed);
                    clcd_print("      Open      ", LINE1(0));
                    clcd_print("     Tera Term    ", LINE2(0));
                    download_log(key);
                    __delay_ms(2000);
                    control_flag = MAINMENU_SCREEN;
                    reset_flag = RESET_LOGIN_MENU;
                    break;

                case 3: //Change Password 

                    clear_screen();
                    control_flag = CHANGE_PASSWRD;
                    reset_flag = RESET_PASSWORD;
                    TMR2ON = 1;
                    break;

                case 4: //Set Time 

                    clear_screen();
                    log_event("ST", speed);
                    control_flag = SET_TIME;
                    reset_flag = RESET_TIME;
                    break;
            }
        }

        else if (key == SW4_LP && control_flag == VIEW_LOG)
        {
            control_flag = MAINMENU_SCREEN;
            clear_screen();
        }

        else if (key == SW4_LP&& control_flag == CHANGE_PASSWRD)
        {
            control_flag = MAINMENU_SCREEN;
            clear_screen();
        }

        else if (key == SW5_LP&& control_flag == MAINMENU_SCREEN)
        {
            control_flag = DASHBOARD_SCREEN;
            clear_screen();
        }  
    switch(control_flag)
    {
        case DASHBOARD_SCREEN:
        {
            display_dashboard(event,speed);
            break;
        }
        case LOGIN_SCREEN:
        {
            switch(login(key,reset_flag))
            {
                case RETURN_BACK:
                {
                    clear_screen();
                    control_flag=DASHBOARD_SCREEN;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2ON=0;
                    break;                  
                }
                case LOGIN_SUCCESS:
                {
                    clear_screen();
                    control_flag=MAINMENU_SCREEN;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    reset_flag = RESET_MENU;
                    TMR2ON=0;
                    break;      
                }
            }
            break;
        }
        case MAINMENU_SCREEN: //login menu screen 

                switch (menu_screen(key, reset_flag))
                {
                    case RETURN_BACK: //login failed 
                        clear_screen();
                        control_flag = DASHBOARD_SCREEN; //go back to dashboard 
                        TMR2ON = 0; //turn off the timer2 
                        break;

                    case 0: // return 0, i.e menu_pos is 0 
                        menu_pos = 0;
                        break;

                    case 1: // return 1, i.e menu_pos is 1 
                        menu_pos = 1;
                        break;

                    case 2: //return 2, i.e menu_pos is 2 
                        menu_pos = 2;
                        break;

                    case 3: //return 3, i.e menu_pos is 3 
                        menu_pos = 3;
                        break;

                    case 4: //return 4, i.e menu_pos is 4 
                        menu_pos = 4;
                        break;
                }
                break;

            case VIEW_LOG: //View Log 

                view_log(key, reset_flag); //function call 
                break;

            case CLEAR_LOG: //Clear Log 

                if (clear_log(reset_flag) == TASK_SUCCESS)
                    __delay_ms(1000);

                if (reset_flag == RESET_MEMORY)
                    log_event("CL", speed);

                control_flag = MAINMENU_SCREEN; //go back to login menu 
                reset_flag = RESET_LOGIN_MENU;
                clear_screen();
                break;

            case CHANGE_PASSWRD: //Change Password 

                switch (change_password(key, reset_flag))
                {
                    case TASK_SUCCESS:
                        __delay_ms(1000);
                        log_event("CP", speed);
                        control_flag = MAINMENU_SCREEN; //go back to login menu 
                        reset_flag = RESET_LOGIN_MENU;
                        clear_screen();
                        break;

                    case RETURN_BACK:
                        control_flag = DASHBOARD_SCREEN; //go back to dashboard 
                        reset_flag = RESET_LOGIN_MENU;
                        break;
                }
                break;

            case SET_TIME: //Set Time 

                if (set_time(key, reset_flag) == TASK_SUCCESS)
                {
                    control_flag = MAINMENU_SCREEN; //go back to login menu 
                    reset_flag = RESET_LOGIN_MENU;
                    clear_screen();
                    continue;
                }
                break;
        }
    
    reset_flag=RESET_NOTHING;
    }
    return;
}
