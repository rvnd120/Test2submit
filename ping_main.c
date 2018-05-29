#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "ping.h"




int put_index =0 ;
int get_index =0;
int buffer_size = 16;
unsigned char data = 0;
unsigned char buf = 0;

ppbuf_t *test;

void ping_pong_init(void)
{
        unsigned char ppbuf_mem[33]={0};
        test->buffer_data = ppbuf_mem;
        test->ping = 1;
        test->pong = 0;
        test->full_signal = false;
}

uint8_t ppbuf_insert(uint8_t size)
{
        uint8_t ret =0 ;
        uint8_t *ptr;

        if(test == NULL || data==NULL || size ==0)
        {
                ret = -1;
        }
        else
        {
                if(size > (buffer_size - put_index))
                {
                        ret = -1;
                }
                else
                {
                        //printf("before mem %d\n",put_index);
                        int mem_position = ((test->ping) * (buffer_size)) + put_index;
                        ptr = test->buffer_data;
                        //printf("%p\n",&ptr[mem_position]);
                        //printf("memcpy %p\n",memcpy(&ptr[mem_position],(unsigned char*)data,size));
                        ptr[mem_position]=*(unsigned char*) data;
			printf("data %d\n",ptr[mem_position]);
                        put_index = (put_index) + 1;
                        //printf("size %d\n", size);
                        //printf("index %d\n",put_index);
                        test->full_signal = ((put_index >= buffer_size) ? true:false);
                        ret = 0;
                }
        }
        return ret;
}

uint8_t ppbuf_remove(uint8_t size)
{
        uint8_t ret = 0;
        uint8_t* ptr;

        if(test==NULL || data == NULL || size == 0)
        {
                ret=-1;
        }
        else
        {
                if(size > (buffer_size - get_index))
                {
                        ret = -1;
                }

                else
                {
                        int mem_position = ((test->pong) * (buffer_size)) + get_index;
                        ptr = test->buffer_data;
			
                        memcpy((unsigned char*)data,&ptr[mem_position],size);
                        get_index = get_index + size;
                        ret = 0;
                }
        }

        return ret;
}


bool ppbuf_full_signal( bool consume)
{
        bool ret = (test != NULL ? test->full_signal : false);

        if((consume != false) && (test!= NULL) && (ret != false))
        {
                test->full_signal = false;

                /* swap the buffer switches */
                test->ping = test->ping ^ test->pong;
                test->pong = test->pong ^ test->ping;
                test->ping = test->ping ^ test->pong;

                /* resets the buffer position */
                get_index= 0;
                put_index = 0;
        }

        return(ret);
}
                       

int main(void)
{

	test=(ppbuf_t*)malloc(sizeof(ppbuf_t));
	ping_pong_init();
	
	while(ppbuf_full_signal(false)!=true)
	{
		ppbuf_insert( sizeof(data));
		printf("data inserted on ping buffer: %hu", data);
		data += 2;
		ppbuf_remove(sizeof(buf));
		printf("		data got from pong buffer: %hu \n\r", buf);
	}

	ppbuf_full_signal(true);
	printf("SWAPPING BUFFERS! \n\r");
	data = 1;

	while(ppbuf_full_signal(false)!=true)
	{
		ppbuf_insert( sizeof(data));
		printf("data inserted on ping buffer: %hu", data);
		data += 2;
		ppbuf_remove(sizeof(buf));
		printf("                data got from pong buffer: %hu \n\r", buf);
	}

	return 0;
}

		
