#ifndef CAR_BLACK_BOX_H
#define CAR_BLACK_BOX_H
void display_dashboard(unsigned char event[],unsigned char speed);
void log_event(unsigned char event[],unsigned char speed);
void clear_screen();
unsigned char login(unsigned char key,unsigned char reset_flag);
unsigned char menu_screen(unsigned char key,unsigned char reset_flag);
unsigned char view_log(unsigned char key,char reset_flag);
unsigned char clear_log(char reset_memory);
unsigned char download_log(unsigned char key);
unsigned char change_password(unsigned char key,unsigned char reset_pwd);
unsigned char set_time(unsigned char key,unsigned char reset_time);
#endif	

