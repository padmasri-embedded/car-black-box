/*
 * File:   car_black_box.c
 * Author: padmasri
 *
 * Created on 19 March, 2025, 12:25 AM
 */
#include "main.h"
unsigned char clock_reg[3];
char time[7];// "HH:MM:SS"
char log[11];//HHMMEVSP
char log_pos;
unsigned char sec;
unsigned char return_time=5;
int event_count = -1;
unsigned char* menu_array[] = {"View_Log", "Clear_Log", "Download_log", "change_passwrd", "set_time"};
 void get_time()
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD 
    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD 
    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD 
    // HH 
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    // MM 
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';
    // SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
}
void display_time()
{
    get_time();
    clcd_putch(time[0],LINE2(2));
    clcd_putch(time[1],LINE2(3));
    clcd_putch(':',LINE2(4));
    clcd_putch(time[2],LINE2(5));
    clcd_putch(time[3],LINE2(6));
    clcd_putch(':',LINE2(7));
    clcd_putch(time[4],LINE2(8));
    clcd_putch(time[5],LINE2(9));
}
//display dashboard function definition
void display_dashboard(unsigned char event[],unsigned char speed)
{
    clcd_print("TIME     E  SP",LINE1(2));
    //display time
    display_time();
    //display event
    clcd_print(event,LINE2(11));
    //display speeds
    clcd_putch(speed /10+'0' ,LINE2(14));
    clcd_putch(speed %10+'0' ,LINE2(15));
    
}
void store_event()
{
    char addr;
    if(log_pos==10)
    {
        log_pos=0;
    }
    addr=0x05 + log_pos*10;
    ext_eeprom_24C02_str_write(addr,log);
    log_pos++;
}
void log_event(unsigned char event[],unsigned char speed)
{
    get_time();
    strncpy(log,time,6);//time
    strncpy(&log[6],event,2);//event
    //speed
    log[8]=speed/10+'0';
    log[9]=speed%10+'0';
    log[10]='\0';
    store_event();
    
}
//function for login screen
unsigned char login(unsigned char key,unsigned char reset_flag)
{
    static char npassword[4]; /*to store new password  */
    char spassword[4]; /*to store system password  */
    static char i;
    static unsigned char attempts_rem = 3;


    if (reset_flag == RESET_PASSWORD) /* reset the user entered password */
    {
        attempts_rem = 3; /* giving fresh 3 attempts */

        /* storing password with null */
        npassword[0] = '\0';
        npassword[1] = '\0';
        npassword[2] = '\0';
        npassword[3] = '\0';

        i = 0;
        key = 0xFF; /* updating key value with some value other than SW1-SW6 */
        return_time = 5; /* waiting 5 sec for each password character entry */
    }

    /*____________________________________________________________________*/

    if (return_time == 0)
    {
        return RETURN_BACK; /* return back to dashboard */
    }
    __delay_ms(50);

    if (key == SW4 && i < 4) /*storing new password as 1*/
    {
        npassword[i] = '1';
        clcd_putch('*', LINE2(4+ i)); // display star(*)
        i++;
        return_time = 5;
    }
    else if (key == SW5 && i < 4) /* storing new password as 0*/
    {
        npassword[i] = '0';
        clcd_putch('*', LINE2(4 + i)); // display star(*)
        i++;
        return_time = 5;
    }
    if (i == 4) 
    {
        //read the system password 
        for (int j = 0; j < 4; j++)
        {
            spassword[j] = ext_eeprom_24C02_read(j);
        }

        //comparing the password with the system password 
        if (strncmp(spassword, npassword, 4) == 0)
        {
            clear_screen();
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            __delay_ms(100);
            clcd_print("LOGIN SUCCESS",LINE1(1));
            __delay_ms(3000);
            return LOGIN_SUCCESS;
            //change to menu screen
        }
        else
        {
            attempts_rem--; //reducing the remaining attempts 

            if (attempts_rem == 0) //when no attempts remain 
            {
                sec = 60; 

                clear_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("You Are  Blocked", LINE1(0));
                clcd_print("Wait for", LINE2(0));
                sec=60;

                while (sec) 
                {
                    /* printing the mins */
                    clcd_putch(((sec / 60) / 10) + '0', LINE2(11));
                    clcd_putch(((sec / 60) % 10) + '0', LINE2(12));
                }
                attempts_rem = 3; /* giving fresh 3 attempts again */
            }
            else //attempts
            {

                clear_screen();
                clcd_print(" Wrong Password ", LINE1(0));
                clcd_putch(attempts_rem + '0', LINE2(0));
                clcd_print(" Attempt Remain", LINE2(1));
                __delay_ms(3000);
            }
            clear_screen();
            clcd_print(" Enter Password ", LINE1(0));
            i = 0;
            return_time = 5;
            clcd_write(LINE2(4), INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
        }
    }

    return 0xFF;
}
void clear_screen()
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}
//display menu screen    
unsigned char menu_screen(unsigned char key,unsigned char reset_flag)
{
     static char menu_pos;

    if (return_time == 0)
    {
        return RETURN_BACK; //return back to dashboard 
    }
    if (reset_flag == RESET_MENU) //default login menu screen position 
    {
        return_time =5;
        menu_pos = 0;
        clear_screen();
    }
    if (key == SW4 && menu_pos > 0) //scrolling up 
    {
        return_time =5;
        menu_pos--;
        clear_screen();
    }

    else if (key == SW5 && menu_pos <= 3)// scrolling down 
    {
        return_time =5;
        menu_pos++;
        clear_screen();
    }

    // displaying the star(*) in front of the menu options 

    if (menu_pos < 4)
    {
        clcd_putch('*', LINE1(0));
        clcd_print(menu_array[menu_pos], LINE1(2));
        clcd_print(menu_array[menu_pos + 1], LINE2(2));
    }

    else if (menu_pos == 4)
    {

        clcd_print(menu_array[menu_pos - 1], LINE1(2));
        clcd_print(menu_array[menu_pos], LINE2(2));
        clcd_putch('*', LINE2(0));
    }

    return menu_pos;
}
 //view log function  
unsigned char view_log(unsigned char key,char reset_flag)
{
    //eeprom write
    //variable declaration
    unsigned char log_read[11];
    unsigned char eeprom_read[10];
    unsigned char addr;
    //track the current log index
    static int log_c = 0;
    return_time=5;
    
    //check the current index within a valid range
    if(log_c==10)//reset the log_c if it ie equal to 10
    {
        log_c=0;
    }
    //calculate the address of eeprom for current log
    addr = (unsigned char) (0x05 + log_c * 10);
    
    if(key==SW4 &&log_c<9)//move to the next log
    {
        clear_screen();
        log_c++;
    }
    else if(key==SW5&& log_c>0)//move to the previous log
    {
        clear_screen();
        log_c--;
    }
    //read log data from eeprom
    for(int i=0;i<10;i++)
    {
        eeprom_read[i]=ext_eeprom_24C02_read(addr+i);
    }
    
    //copy data of eeprom to log_read
    for(int i=0;i<10;i++)
    {
        log_read[i]=eeprom_read[i];
    }
    log_read[10]='\0';
    
    //diaplay on clcd
    clcd_print("#", LINE1(0));
    clcd_putch(log_c + '0', LINE2(0));
    clcd_print("TIME  E SP", LINE1(6));
    clcd_putch(log_read[0], LINE2(2));
    clcd_putch(log_read[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(log_read[2], LINE2(5));
    clcd_putch(log_read[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(log_read[4], LINE2(8));
    clcd_putch(log_read[5], LINE2(9));
    clcd_putch(log_read[6], LINE2(11));
    clcd_putch(log_read[7], LINE2(12));
    clcd_putch(log_read[8], LINE2(14));
    clcd_putch(log_read[9], LINE2(15));
    __delay_us(500);     
}

//clear log function
unsigned char clear_log(char reset_memory)
{
    clcd_print("Logs cleared", LINE1(2));
    clcd_print("successfully", LINE2(2));

    //clearing all the logs
    if (reset_memory == RESET_MEMORY)
    {
        //updating the flags 
        log_pos = 0;
        event_count = -1;

        return TASK_SUCCESS;
    }

    return TASK_FAILURE;
    

}

//download_log function
unsigned char download_log(unsigned char key)
{
    int index = -1;
    char log[11];
    log[10] = 0;
    int position = 0;
    unsigned char addr;

    if (event_count == -1) // if no logs available 
    {
        puts("No logs available");
    }
    else //displaying all logs available 
    {
        puts("Logs :");
        putchar('\n');
        puts("#     Time        Event       Speed");
        putchar('\n');
        putchar('\r');

        while (index < event_count)
        {
            position = index + 1;
            index++; //incrementing to next log 

            for (int i = 0; i < 10; i++)
            {

                addr = position * 10 + 5 + i; // updating the address with that of event data 
                log[i] = ext_eeprom_24C02_read(addr); //reading the event data 
            }
            //printing index 
            putchar(index + '0');
            puts("   ");

            //printing hours value 
            putchar(log[0]);
            putchar(log[1]);
            putchar(':');

            //printing minute  value 
            putchar(log[2]);
            putchar(log[3]);
            putchar(':');

            //printing second  value
            putchar(log[4]);
            putchar(log[5]);
            puts("      ");

            //printing event  character
            putchar(log[6]);
            putchar(log[7]);
            puts("            ");

            //printing speed value 
            putchar(log[8]);
            putchar(log[9]);
            putchar('\n');
            putchar('\r');
        }
    }
      
}

//password change function
unsigned char change_password(unsigned char key,unsigned char reset_pwd)
{   
    static char new_pwd[9];
    static int pwd_pos = 0;
    static char pwd_changed = 0;
    static unsigned char toggle_cursor = 0;
    static unsigned int blink_delay = 0;

    // cursor blinking 
    if (blink_delay++ == 5)
    {
        blink_delay = 0;
        toggle_cursor = !toggle_cursor;
    }
    //checking reset flag and rest change password 
    if (reset_pwd == RESET_PASSWORD)
    {
        strncpy(new_pwd, "    ", 4);
        pwd_pos = 0;
        pwd_changed = 0;
        return_time = 5;
    }
    if (!return_time)
        return RETURN_BACK;
    if (pwd_changed)
        return TASK_FAILURE;
    if (pwd_pos < 4)
    {
        clcd_print("Enter new pwd:  ", LINE1(0));

        // blinking the cursor 
        if (toggle_cursor == 0)
        {
            clcd_putch((unsigned char) 0xFF, LINE2(pwd_pos));
        }
        else
        {
            clcd_putch(' ', LINE2(pwd_pos));
        }
    }
    else if (pwd_pos > 3 && pwd_pos < 8)
    {
        clcd_print("Re-enter new pwd", LINE1(0));
        if (toggle_cursor == 0)
        {
            clcd_putch((unsigned char) 0xFF, LINE2(pwd_pos));
        }
        else
        {
            clcd_putch(' ', LINE2(pwd_pos));
        }
    }
    switch (key)
    {
        case SW5: //storing new password as 0
            new_pwd[pwd_pos] = '0';
            clcd_putch('*', LINE2(pwd_pos));
            pwd_pos++;
            return_time = 5;
            if (pwd_pos == 4)
                clcd_print("                 ", LINE2(0));
            break;

        case SW4: //storing new password as 1
            new_pwd[pwd_pos] = '1';
            clcd_putch('*', LINE2(pwd_pos));
            pwd_pos++;
            return_time = 5;
            if (pwd_pos == 4)
                clcd_print("                 ", LINE2(0));
            break;
    }
    if (pwd_pos == 8)
    {
        if (strncmp(new_pwd, &new_pwd[4], 4) == 0) //if both password matched 
        {
            //successfully changing password 
            new_pwd[8] = 0;
            ext_eeprom_24C02_str_write(0x00, &new_pwd[4]); //storing new password 
            pwd_pos++;
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            clcd_print("Password changed", LINE1(0));
            clcd_print("successfully ", LINE2(2));
            pwd_changed = 1;
            __delay_ms(1000);
            return TASK_SUCCESS;
        }
        else
        {
            //displaying password change fail 
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            clcd_print("Password  change", LINE1(0));
            clcd_print("failed", LINE2(5));
            pwd_changed = 1;
            __delay_ms(1000);

            return TASK_SUCCESS;
        }
    }

    return TASK_FAILURE;
}

//set_time function
unsigned char set_time(unsigned char key,unsigned char reset_time)
{
   static unsigned int new_time[3];
    static unsigned int blink_pos;
    static unsigned char wait;
    static unsigned char blink;
    static char t_done = 0;
    char buffer;
    if (reset_time == RESET_TIME)
    {
        get_time(); //get the time 

        //Storing values of new time hours 
        new_time[0] = (time[0] & 0x0F) * 10 + (time[1] & 0x0F);

        //Storing values of new time minute
        new_time[1] = (time[2] & 0x0F) * 10 + (time[3] & 0x0F);

        //Storing values of new time second 
        new_time[2] = (time[4] & 0x0F) * 10 + (time[5] & 0x0F);

        clcd_print("Time (HH:MM:SS)", LINE1(0));

        //updating the flags 
        blink_pos = 2;
        wait = 0;
        blink = 0;
        t_done = 0;
        key = ALL_RELEASED;
    }
    if (t_done)
        return TASK_FAILURE;

    switch (key)
    {
        case SW4: //incrementing the value 
            new_time[blink_pos]++;
            break;

        case SW5: //changing the field 
            blink_pos = (blink_pos + 1) % 3;
            break;

        case SW4_LP: // storing the new time 

            get_time(); //get the time 

            buffer = ((new_time[0] / 10) << 4) | new_time[0] % 10; //set time fetching from rtc 
            clock_reg[0] = (clock_reg[0] & 0xC0) | buffer; //assigned fetched time to clock register 
            write_ds1307(HOUR_ADDR, clock_reg[0]); //storing time in ds1307 


            buffer = ((new_time[1] / 10) << 4) | new_time[1] % 10; //set time fetching from rtc 
            clock_reg[1] = (clock_reg[1] & 0x80) | buffer; //assigned fetched time to clock register 
            write_ds1307(MIN_ADDR, clock_reg[1]); //storing time in ds1307 


            buffer = ((new_time[2] / 10) << 4) | new_time[2] % 10; //set time fetching from rtc 
            clock_reg[2] = (clock_reg[2] & 0x80) | buffer; //assigned fetched time to clock register 
            write_ds1307(SEC_ADDR, clock_reg[2]); //storing time in ds1307 


            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            clcd_print("Time changed", LINE1(2));
            clcd_print("Successfully", LINE2(2));

            t_done = 1;
            __delay_ms(1000);
            return TASK_SUCCESS;
    }
    // conditions for roll over 
    if (new_time[0] > 23)
        new_time[0] = 0;
    if (new_time[1] > 59)
        new_time[1] = 0;
    if (new_time[2] > 59)
        new_time[2] = 0;
    if (wait++ == 1)
    {
        wait = 0;
        blink = !blink;
        //logic to blink at the current pos
        if (blink)
        {
            switch (blink_pos)
            {
                case 0:
                    clcd_print("  ", LINE2(0));
                    __delay_ms(160);
                    break;
                case 1:
                    clcd_print("  ", LINE2(3));
                    __delay_ms(160);
                    break;
                case 2:
                    clcd_print("  ", LINE2(6));
                    __delay_ms(160);
                    break;
            }
        }
    }
    // Displaying hours field 
    clcd_putch(new_time[0] / 10 + '0', LINE2(0));
    clcd_putch(new_time[0] % 10 + '0', LINE2(1));
    clcd_putch(':', LINE2(2));

    //Displaying mins field 
    clcd_putch(new_time[1] / 10 + '0', LINE2(3));
    clcd_putch(new_time[1] % 10 + '0', LINE2(4));
    clcd_putch(':', LINE2(5));

    //Displaying secs field 
    clcd_putch(new_time[2] / 10 + '0', LINE2(6));
    clcd_putch(new_time[2] % 10 + '0', LINE2(7));

    return TASK_FAILURE;
   
}
 

   
   

    

