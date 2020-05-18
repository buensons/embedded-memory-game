#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <gpiod.h>

#ifndef	CONSUMER
#define	CONSUMER "its_me"
#endif

int* run_led_sequence(struct gpiod_chip* chip, int length);
int* get_user_input(struct gpiod_chip* chip, int length);
void check_and_print(int* led, int* user, int n);
void cleanup(const char * msg, struct gpiod_line_bulk* bulk);

int main(int argc, char ** argv) {

	if(argc != 2) {
		printf("usage: memory_game <sequence_length>");	
		return 1;
	}

    char* chipname = "gpiochip1";
	struct gpiod_chip* chip;
    int length = atoi(argv[1]);

    chip = gpiod_chip_open_by_name(chipname);
	if (!chip) {
		perror("Open chip failed\n");
		return 1;
	}
	
	printf("Memorize the sequence of blinking LEDs [24-27]\n");
    printf("Press Enter when you are ready...\n");
    getchar();

	int* led_sequence = run_led_sequence(chip, length);
    
    printf("Now, recreate the the previous sequence with buttons 12-15\n");

    int* user_sequence = get_user_input(chip, length);

	if(user_sequence != NULL) {
		check_and_print(led_sequence, user_sequence, length);
	}

	gpiod_chip_close(chip);
	free(led_sequence);
	free(user_sequence);
	return 0;
}

int* run_led_sequence(struct gpiod_chip* chip, int length) {
	struct gpiod_line* line;
	int line_num, val, ret;
	int* sequence;
	srand(time(NULL));

	if((sequence = malloc(length * sizeof(int))) == NULL) {
		perror("malloc failed\n");
		exit(1);
	}

	for(int i = 0; i < length; i++) {
        line_num = rand() % 4 + 24;
		sequence[i] = line_num - 24;

        line = gpiod_chip_get_line(chip, line_num);
	    if (!line) {
		    perror("Get line failed\n");
		    gpiod_chip_close(chip);
			free(sequence);
			exit(1);
	    }

        ret = gpiod_line_request_output(line, CONSUMER, 0);
	    if (ret < 0) {
		    perror("Request line as output failed\n");
		    gpiod_line_release(line);
			free(sequence);
			exit(1);
	    }

        /* Blink */
	    val = 1;
	    for (int i = 0; i < 2; ++i) {
		    ret = gpiod_line_set_value(line, val);
		    if (ret < 0) {
			    perror("Set line output failed\n");
			    gpiod_line_release(line);
				free(sequence);
				exit(1);
		    }
		    printf("Output %u on line #%u\n", val, line_num);
		    sleep(1);
		    val = !val;
	    }

		gpiod_line_release(line);
    }

	return sequence;
}

int* get_user_input(struct gpiod_chip* chip, int length) {
	int* sequence;
	struct gpiod_line_bulk lines;
	struct gpiod_line_bulk event_bulk;
	struct gpiod_line * line;
	int nums[] = {12,13,14,15};
	struct timespec t = {1,0};
	struct timeval previous;
	struct timeval current;
	int values[4];
	int counter = 0;
	int ret;

	gettimeofday(&previous, NULL);

	if((sequence = malloc(length * sizeof(int))) == NULL) {
		perror("malloc failed\n");
		return NULL;
	}

	ret = gpiod_chip_get_lines(chip, nums, 4, &lines);
	if(ret < 0) {
		cleanup("chip get lines\n", &lines);
		return NULL;
	}

	while(counter < length) {

		ret = gpiod_line_request_bulk_falling_edge_events(&lines, CONSUMER);
		if(ret < 0) {
			cleanup("Request Fallin Edge Error\n", &lines);
			return NULL;
		}

		ret = gpiod_line_event_wait_bulk(&lines, &t, &event_bulk);
		
		if(ret < 0) {
			cleanup("Wait for bulk event\n", &lines);
			return NULL;
		} else if(ret > 0) {
			if(&event_bulk == NULL) continue;

			gettimeofday(&current, NULL);

			if((current.tv_sec - previous.tv_sec) * 1000000 + current.tv_usec - previous.tv_usec < 250000) {
				gpiod_line_release_bulk(&lines);
				previous = current;
				continue;
			}

			previous = current;
			
			line = gpiod_line_bulk_get_line(&event_bulk, 0);
			int line_number = gpiod_line_offset(line);
			printf("FALLING EDGE EVENT %d\n", line_number);
			sequence[counter++] = line_number - 12;

		} else {
			printf(".\n");
		}
	
		gpiod_line_release_bulk(&lines);
	}

	return sequence;
}

void cleanup(const char * msg, struct gpiod_line_bulk* bulk) {
	gpiod_line_release_bulk(bulk);
	perror(msg);
}

void check_and_print(int* led, int* user, int n) {
	bool is_correct = true;

	printf("LED sequence: ");
	for(int i = 0; i < n; i++) {
		printf("%d ", led[i]);
	}

	printf("\nYour sequence: ");
	for(int i = 0; i < n; i++) {
		printf("%d ", user[i]);
		if(user[i] != led[i]) {
			is_correct = false;
		}
	}

	if(is_correct) {
		printf("\nSUCCESS!");
	} else {
		printf("\nFAILURE!");
	}
}