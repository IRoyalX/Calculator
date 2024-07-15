#include <mega16.h>
#include <stdio.h>
#include <delay.h>
#include <string.h>

#define mx 40
#define flt 7
#define RS PORTB0
#define EN PORTB1
#define A PORTA
#define B PORTB


void lcd_comm(char comm)
{
    A = comm;
    B &= ~(1<<RS);
    B |= (1<<EN);
    delay_ms(1);
    B &= ~(1<<EN);
}
void lcd_data(char data)
{
    A = data;
    B |= (1<<RS);
    B |= (1<<EN);
    delay_ms(1);
    B &= ~(1<<EN);
}

char keypad()
{

    int i,j;
    char keys[4][4] = {{'7','8','9','/'},{'4','5','6','*'},{'1','2','3','-'},{'c','0','=','+'}}; 
    while(1)
    {   
        for(i = 0;i<4;i++)
        {    
            PORTC = ~(1<<i);
            if(!PINC.4){j = 0;while(!PINC.4);return keys[i][j];}
            if(!PINC.5){j = 1;while(!PINC.5);return keys[i][j];}
            if(!PINC.6){j = 2;while(!PINC.6);return keys[i][j];}
            if(!PINC.7){j = 3;while(!PINC.7);return keys[i][j];}
        }
        if(!PIND.0){while(!PIND.0);return '(';}
        if(!PIND.1){while(!PIND.1);return ')';}
        if(!PIND.2){while(!PIND.2);return 'b';}     
    }
}    
void calc()
{
    int i = 0, pr = 0, st = 0, ps = 0, ct;
    float x[mx/2+1];
    unsigned char str[mx+1] = "", post[mx+1] = "", stack[mx/2+1] = "",ch = '', lst = '';
    lcd_comm(0x01);
    while(ch != '=' && ch != 'c')
    {   
        ch = keypad();
        if(i != mx || ch == 'b')
            switch(ch)
            {   
                
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    if(lst != ')')
                    {
                        lcd_data(ch);
                        str[i++] = ch;
                    }
                    else
                    {
                      lcd_data('*');
                      lcd_data(ch);
                      str[i++] = '*';
                      str[i++] = ch;
                    }
                    lst = 'i';
                    break;
                case '+':case '-':case '*':case '/':
                    if(i != 0 && lst != '(' && lst != 'o')
                    {
                        lcd_data(ch);
                        str[i++] = ch;
                        lst = 'o';
                    }
                    else if(lst == 'o')
                    {
                        lcd_comm(0x10);
                        lcd_data(ch);
                        str[i-1] = ch;
                        lst = 'o';
                    }
                    break;
                case '(':                
                    if(i == 0 || lst == '(' || lst == 'o')
                    {
                       lcd_data(ch);
                       str[i++] = ch; 
                    }
                    else
                    {
                        lcd_data('*');
                        lcd_data(ch);
                        str[i++] = '*';
                        str[i++] = '(';
                    }
                    pr++;
                    lst = '(';
                    break;
                case ')':
                    if(pr != 0 && lst != '(' && lst != 'o')
                    {
                        lcd_data(ch);
                        str[i++] = ch;
                        pr--;
                        lst = ')';
                    }
                    else if(lst == '(')
                    {
                        lcd_comm(0x10);
                        lcd_data('');
                        lcd_comm(0x10);
                        pr--;
                        i -= 2;
                        switch(lst = str[i++])
                            {
                                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                    lst = 'i';
                                    break;
                                case '+':case '-':case '*':case '/':
                                    lst = 'o';
                                    break;
                                default:break;
                            }
                    }
                    break;
                case 'b':
                    if(i != 0)
                    {
                        lcd_comm(0x10);
                        lcd_data('');
                        lcd_comm(0x10);
                        if(str[i-1] == '(')
                            pr--;
                        else if(str[i-1] == ')')
                            pr++;    
                        if(i == 1)
                            lst = --i;
                        else
                        {
                            i -= 2;
                            lst = str[i++];
                            switch(lst)
                            {
                                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                    lst = 'i';
                                    break;
                                case '+':case '-':case '*':case '/':
                                    lst = 'o';
                                    break;
                                default:break;
                            }
                        }
                    }
                    break; 
                default:break;   
            }
    }
    if(ch == '=')
    {  
       str[i] = '\0';
       while(i != 0 && (str[i-1] == '(' || str[i-1] == '+' || str[i-1] == '-' || str[i-1] == '*' || str[i-1] == '/'))
       {
            if(str[i-1] == '(')
                pr--;
            str[--i] = '\0';
       }
       while(pr != 0)
       {
         str[i++] = ')';
         pr--; 
       }
       if(i != 0)
       {    
            for(ct = 0;ct < i;ct++)
                switch(str[ct])
                {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':    
                        if(post[ps-1] == '+' || post[ps-1] == '-' || post[ps-1] == '*' || post[ps-1] == '/')
                            post[ps++] = ' ';
                        post[ps++] = str[ct];
                        break;
                    case '+':case '-':case '*':case '/':
                        if(post[ps-1] != '+' && post[ps-1] != '-' && post[ps-1] != '*' && post[ps-1] != '/')
                            post[ps++] = ' ';
                        stack[st++] = str[ct];
                        break;
                    case ')':
                        if(st != 0)
                            post[ps++] = stack[--st];
                        break;
                    default:break;
                }
            while(st != 0)
            post[ps++] = stack[--st];
            
            //lcd_comm(0x01);
            //lcd_str(post);
            //while(1);
            i = 0;
            x[i] = 0;
            for(ct = 0;ct < ps;ct++)
                switch(post[ct])
                {   
                    case ' ':x[++i] = 0;break;
                    case '0':x[i] = x[i] * 10;break;
                    case '1':x[i] = x[i] * 10 + 1;break;
                    case '2':x[i] = x[i] * 10 + 2;break;
                    case '3':x[i] = x[i] * 10 + 3;break;
                    case '4':x[i] = x[i] * 10 + 4;break;
                    case '5':x[i] = x[i] * 10 + 5;break;
                    case '6':x[i] = x[i] * 10 + 6;break;
                    case '7':x[i] = x[i] * 10 + 7;break;
                    case '8':x[i] = x[i] * 10 + 8;break;
                    case '9':x[i] = x[i] * 10 + 9;break;
                    case '+':x[--i] += x[i+1];break; 
                    case '-':x[--i] -= x[i+1];break;
                    case '*':x[--i] *= x[i+1];break;
                    case '/':x[--i] /= x[i+1];break;
                    default:break;   
                } 
            
            lcd_comm(0x01);  
            st = x[0];
            x[0] -= st;
            ct = 1;
            if(x[0] < 0 || st < 0){st *= -1,x[0] *= -1,ct++,lcd_data('-');}
            for(i = 10;st/i > 0;i *= 10)ct++;
            for(i /= 10;i != 0; i /= 10)
            { 
                switch(st/i)
                {
                    case 0:lcd_data('0');break;
                    case 1:lcd_data('1');break;
                    case 2:lcd_data('2');break;
                    case 3:lcd_data('3');break;
                    case 4:lcd_data('4');break;
                    case 5:lcd_data('5');break;
                    case 6:lcd_data('6');break;
                    case 7:lcd_data('7');break;
                    case 8:lcd_data('8');break;
                    case 9:lcd_data('9');break;
                    default:break;  
                }
                st %= i;
            }
            x[1] = x[0];
            for(i = flt;i > 0;i--)
                x[1] *= 10;
            pr = x[1];
            if(pr)
                if(mx-(mx-flt)-(ct++) > 0)
                {   
                    lcd_data('.');
                    st = 0;
                    for(i = flt;i > 0;i--)
                    {    
                        pr = x[0] * 10;
                        x[0] = x[0] * 10 - pr;
                         switch(pr)
                        {
                            case 0:st++;break;
                            case 1:for(;st > 0;st--)lcd_data('0');lcd_data('1');break;
                            case 2:for(;st > 0;st--)lcd_data('0');lcd_data('2');break;
                            case 3:for(;st > 0;st--)lcd_data('0');lcd_data('3');break;
                            case 4:for(;st > 0;st--)lcd_data('0');lcd_data('4');break;
                            case 5:for(;st > 0;st--)lcd_data('0');lcd_data('5');break;
                            case 6:for(;st > 0;st--)lcd_data('0');lcd_data('6');break;
                            case 7:for(;st > 0;st--)lcd_data('0');lcd_data('7');break;
                            case 8:for(;st > 0;st--)lcd_data('0');lcd_data('8');break;
                            case 9:for(;st > 0;st--)lcd_data('0');lcd_data('9');break;
                            default:break;  
                        }
                    }
                }
            while(keypad() != 'c');
       }    
    } 
}

void main(void)
{
    DDRA = 0xFF;
    DDRB |= (1<<RS)|(1<<EN);
    
    DDRC = 0x0F;
    DDRD = 0xF8;
    PORTD = 0x07;
    
    lcd_comm(0x38);
    lcd_comm(0x0c);
    while (1)
    {    
        calc();  
    }
}