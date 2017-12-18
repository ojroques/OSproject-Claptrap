#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>

#include "const.h"
#include "position.h"
#include "sensors.h"
#include "tacho.h"
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"

#ifdef TACHO_DEBUG
coordinate_t coordinate = {60, 30, 90, PTHREAD_MUTEX_INITIALIZER};
volatile int quit_request = 0;   // To stop the position thread
#endif

/* By Olivier.
   Wait for the tachos to stop. */
void wait_tachos() {
    char lsn_state[TACHO_BUFFER_SIZE];
    char rsn_state[TACHO_BUFFER_SIZE];
    uint8_t lsn, rsn;

    if (ev3_search_tacho_plugged_in(LEFT_WHEEL_PORT, 0, &lsn, 0)) {
        if (ev3_search_tacho_plugged_in(RIGHT_WHEEL_PORT, 0, &rsn, 0)) {
            do {
                get_tacho_state(lsn, lsn_state, TACHO_BUFFER_SIZE);
                get_tacho_state(rsn, rsn_state, TACHO_BUFFER_SIZE);
                Sleep(200);
            } while (strcmp("holding", lsn_state) && strcmp("holding", rsn_state));
        }
    }
}

/* By Olivier.
   Wait for the tongs to stop. */
void wait_tongs(int id) {
    char tsn_state[TACHO_BUFFER_SIZE];
    char udsn_state[TACHO_BUFFER_SIZE];
    uint8_t tsn, udsn;

    if (id == UP_DOWN_ID && ev3_search_tacho_plugged_in(UP_DOWN_TONG_PORT, 0, &udsn, 0)) {
        do {
            get_tacho_state(udsn, udsn_state, TACHO_BUFFER_SIZE);
            Sleep(200);
        } while (strcmp("holding", udsn_state));
    }
    else if (id == OPEN_CLOSE_ID && ev3_search_tacho_plugged_in(OPEN_CLOSE_TONG_PORT, 0, &tsn, 0)) {
        do {
            get_tacho_state(tsn, tsn_state, TACHO_BUFFER_SIZE);
            Sleep(200);
        } while (strcmp("holding", tsn_state));
    }
}

//Erwan
void turn_left(float angle) {
    if (angle == 0) {
        return;
    }
    uint8_t lsn;
    uint8_t rsn;
    float rad = angle/360 * 2*M_PI;
    if (ev3_search_tacho_plugged_in(LEFT_WHEEL_PORT, 0, &lsn, 0)) {
        if (ev3_search_tacho_plugged_in(RIGHT_WHEEL_PORT, 0, &rsn, 0)) {
            int max_speed, speed;
            int count_per_rot;
            int rel_pos;
            set_tacho_stop_action_inx(lsn,TACHO_HOLD);
            set_tacho_stop_action_inx(rsn,TACHO_HOLD);
            get_tacho_max_speed(lsn, &max_speed);
            get_tacho_count_per_rot(lsn, &count_per_rot);
            rel_pos = (int)((ROBOT_RADIUS * rad / WHEEL_PERIMETER) * count_per_rot + 0.5);
            speed = (int)((float)max_speed * ROTATION_SPEED / 100.0 + 0.5);
            set_tacho_speed_sp( lsn, speed );
            set_tacho_speed_sp( rsn, speed );
            set_tacho_ramp_up_sp( lsn, 50 );
            set_tacho_ramp_up_sp( rsn, 50 );
            set_tacho_ramp_down_sp( lsn, 50 );
            set_tacho_ramp_down_sp( rsn, 50 );
            set_tacho_position_sp( lsn, -rel_pos );
            set_tacho_position_sp( rsn, rel_pos );
            set_tacho_command_inx( lsn, TACHO_RUN_TO_REL_POS );
            set_tacho_command_inx( rsn, TACHO_RUN_TO_REL_POS );
            update_theta(angle);
        }
    }
}

//Erwan
void turn_right(float angle){
    if (angle == 0) {
        return;
    }
    uint8_t lsn;
    uint8_t rsn;
    float rad = angle/360 * 2*M_PI;
    if (ev3_search_tacho_plugged_in(LEFT_WHEEL_PORT, 0, &lsn, 0)) {
        if (ev3_search_tacho_plugged_in(RIGHT_WHEEL_PORT, 0, &rsn, 0)) {
            int max_speed, speed;
            int count_per_rot;
            int rel_pos;
            set_tacho_stop_action_inx(lsn,TACHO_HOLD);
            set_tacho_stop_action_inx(rsn,TACHO_HOLD);
            get_tacho_max_speed(lsn, &max_speed);
            get_tacho_count_per_rot(lsn, &count_per_rot);
            rel_pos = (int)((ROBOT_RADIUS * rad / WHEEL_PERIMETER) * count_per_rot + 0.5);
            speed = (int)((float)max_speed * ROTATION_SPEED / 100.0 + 0.5);
            set_tacho_speed_sp( lsn, speed );
            set_tacho_speed_sp( rsn, speed );
            set_tacho_ramp_up_sp( lsn, 50 );
            set_tacho_ramp_up_sp( rsn, 50 );
            set_tacho_ramp_down_sp( lsn, 50 );
            set_tacho_ramp_down_sp( rsn, 50 );
            set_tacho_position_sp( lsn, rel_pos );
            set_tacho_position_sp( rsn, -rel_pos );
            set_tacho_command_inx( lsn, TACHO_RUN_TO_REL_POS );
            set_tacho_command_inx( rsn, TACHO_RUN_TO_REL_POS );
            update_theta(-angle);
        }
    }
}

//Erwan
void forward(float distance){
    uint8_t lsn;
    uint8_t rsn;
    if (ev3_search_tacho_plugged_in(LEFT_WHEEL_PORT, 0, &lsn, 0)) {
        if (ev3_search_tacho_plugged_in(RIGHT_WHEEL_PORT, 0, &rsn, 0)) {
            int max_speed, speed;
            int count_per_rot, position_start, current_position, temp;
            int rel_pos;
            set_tacho_stop_action_inx(lsn,TACHO_HOLD);
            set_tacho_stop_action_inx(rsn,TACHO_HOLD);
            get_tacho_max_speed(lsn, &max_speed);
            get_tacho_count_per_rot(lsn, &count_per_rot);
            get_tacho_position(lsn, &position_start);
            rel_pos = (int)((distance / WHEEL_PERIMETER) * count_per_rot + 0.5);
            speed = (int)((float)max_speed * TRANSLATION_SPEED / 100.0 + 0.5);
            set_tacho_speed_sp( lsn, speed );
            set_tacho_speed_sp( rsn, speed );
            set_tacho_ramp_up_sp( lsn, 50 );
            set_tacho_ramp_up_sp( rsn, 50 );
            set_tacho_ramp_down_sp( lsn, 50 );
            set_tacho_ramp_down_sp( rsn, 50 );
            set_tacho_position_sp( lsn, rel_pos );
            set_tacho_position_sp( rsn, rel_pos );
            set_tacho_command_inx( lsn, TACHO_RUN_TO_REL_POS );
            set_tacho_command_inx( rsn, TACHO_RUN_TO_REL_POS );
            get_tacho_position(lsn, &current_position);
            while((current_position-position_start) != rel_pos){
              temp = current_position;
              get_tacho_position(lsn, &current_position);
              update_coordinate(WHEEL_PERIMETER*abs(current_position - temp)/count_per_rot);
              Sleep(100);
            }
        }
    }
}

//Erwan
void backward(float distance){
    uint8_t lsn;
    uint8_t rsn;
    if (ev3_search_tacho_plugged_in(LEFT_WHEEL_PORT, 0, &lsn, 0)) {
        if (ev3_search_tacho_plugged_in(RIGHT_WHEEL_PORT, 0, &rsn, 0)) {
            int max_speed, speed;
            int count_per_rot, position_start, current_position, temp;
            int rel_pos;
            set_tacho_stop_action_inx(lsn,TACHO_HOLD);
            set_tacho_stop_action_inx(rsn,TACHO_HOLD);
            get_tacho_max_speed(lsn, &max_speed);
            get_tacho_count_per_rot(lsn, &count_per_rot);
            get_tacho_position(lsn, &position_start);
            rel_pos = (int)((distance / WHEEL_PERIMETER) * count_per_rot + 0.5);
            speed = (int)((float)max_speed * TRANSLATION_SPEED / 100.0 + 0.5);
            set_tacho_speed_sp( lsn, speed );
            set_tacho_speed_sp( rsn, speed );
            set_tacho_ramp_up_sp( lsn, 50 );
            set_tacho_ramp_up_sp( rsn, 50 );
            set_tacho_ramp_down_sp( lsn, 50 );
            set_tacho_ramp_down_sp( rsn, 50 );
            set_tacho_position_sp( lsn, -rel_pos );
            set_tacho_position_sp( rsn, -rel_pos );
            set_tacho_command_inx( lsn, TACHO_RUN_TO_REL_POS );
            set_tacho_command_inx( rsn, TACHO_RUN_TO_REL_POS );
            get_tacho_position(lsn, &current_position);
            while((current_position-position_start) != rel_pos){
              temp = current_position;
              get_tacho_position(lsn, &current_position);
              update_coordinate(WHEEL_PERIMETER*abs(current_position - temp)/count_per_rot);
              Sleep(100);
            }
        }
    }
}
/* By Olivier
  Stop both wheels. */
void stop_moving() {
    uint8_t lsn;
    uint8_t rsn;
    if (ev3_search_tacho_plugged_in(LEFT_WHEEL_PORT, 0, &lsn, 0)) {
        if (ev3_search_tacho_plugged_in(RIGHT_WHEEL_PORT, 0, &rsn, 0)) {
            set_tacho_command_inx( lsn, TACHO_STOP );
            set_tacho_command_inx( rsn, TACHO_STOP );
        }
    }
}

/* By Olivier
  Stop tongs. */
void stop_tongs() {
    uint8_t udsn;
    uint8_t ocsn;
    if (ev3_search_tacho_plugged_in(UP_DOWN_TONG_PORT, 0, &udsn, 0)) {
        if (ev3_search_tacho_plugged_in(OPEN_CLOSE_TONG_PORT, 0, &ocsn, 0)) {
            set_tacho_command_inx( udsn, TACHO_STOP );
            set_tacho_command_inx( ocsn, TACHO_STOP );
        }
    }
}

//Erwan
// Down: negative value
void down_tongs(uint8_t sonar_id){
    int max_speed, speed;
    uint8_t dsn;
    // Check that the tongs can indeed move down
    if (get_avg_distance(sonar_id, NB_SENSOR_MESURE) < 50) return;

    if (ev3_search_tacho_plugged_in(UP_DOWN_TONG_PORT,0, &dsn, 0 )){
        set_tacho_stop_action_inx(dsn,TACHO_HOLD);
        get_tacho_max_speed(dsn, &max_speed);
        speed = (int)((float)max_speed * UP_DOWN_SPEED / 100.0 + 0.5);
        set_tacho_speed_sp( dsn, speed );
        set_tacho_ramp_up_sp( dsn, 25 );
        set_tacho_ramp_down_sp( dsn, 100 );
        set_tacho_position_sp( dsn, -TONGS_UP_DOWN_DISTANCE );
        set_tacho_command_inx( dsn, TACHO_RUN_TO_REL_POS );
    }
}

//Erwan
// Up: positive value
void up_tongs(uint8_t sonar_id){
    int max_speed, speed;
    uint8_t usn;
    // Check that the tongs can indeed move up
    if (get_avg_distance(sonar_id, NB_SENSOR_MESURE) > 50) return;

    if (ev3_search_tacho_plugged_in(UP_DOWN_TONG_PORT,0, &usn, 0 )){
        set_tacho_stop_action_inx(usn,TACHO_HOLD);
        get_tacho_max_speed(usn, &max_speed);
        speed = (int)((float)max_speed * UP_DOWN_SPEED / 100.0 + 0.5);
        set_tacho_speed_sp( usn, speed );
        set_tacho_ramp_up_sp( usn, 25 );
        set_tacho_ramp_down_sp( usn, 100 );
        set_tacho_position_sp( usn, TONGS_UP_DOWN_DISTANCE );
        set_tacho_command_inx( usn, TACHO_RUN_TO_REL_POS );
    }
}

//Erwan
// Close: positive value
void close_tongs(){
  uint8_t csn;
  if (ev3_search_tacho_plugged_in(OPEN_CLOSE_TONG_PORT,0, &csn, 0 )){
      int max_speed, speed;
      int rel_pos = TONGS_OPEN_CLOSE_DISTANCE;
      set_tacho_stop_action_inx(csn,TACHO_HOLD);
      get_tacho_max_speed(csn, &max_speed);
      speed = (int)((float)max_speed * OPEN_CLOSE_SPEED / 100.0 + 0.5);
      set_tacho_speed_sp( csn, speed );
      set_tacho_ramp_up_sp( csn, 25 );
      set_tacho_ramp_down_sp( csn, 100 );
      set_tacho_position_sp( csn, rel_pos );
      set_tacho_command_inx( csn, TACHO_RUN_TO_REL_POS );
    }
}

//Erwan
// Open: negative value
void open_tongs(){
  uint8_t osn;
  if (ev3_search_tacho_plugged_in(OPEN_CLOSE_TONG_PORT,0, &osn, 0 )){
      int max_speed, speed;
      int rel_pos = -TONGS_OPEN_CLOSE_DISTANCE;
      set_tacho_stop_action_inx(osn,TACHO_HOLD);
      get_tacho_max_speed(osn, &max_speed);
      speed = (int)((float)max_speed * OPEN_CLOSE_SPEED / 100.0 + 0.5);
      set_tacho_speed_sp( osn, speed );
      set_tacho_ramp_up_sp( osn, 25 );
      set_tacho_ramp_down_sp( osn, 100 );
      set_tacho_position_sp( osn, rel_pos );
      set_tacho_command_inx( osn, TACHO_RUN_TO_REL_POS );
    }
}


//Erwan
void turn_gyro_left(float angle, uint8_t gyro_id) {
  if (angle == 0) {
      return;
  }
  int angle_start, current_angle;
  uint8_t lsn;
  uint8_t rsn;

  if (ev3_search_tacho_plugged_in(LEFT_WHEEL_PORT, 0, &lsn, 0)) {
      if (ev3_search_tacho_plugged_in(RIGHT_WHEEL_PORT, 0, &rsn, 0)) {
          int rspeed,lspeed,max_speed;
          set_tacho_stop_action_inx(lsn,TACHO_HOLD);
          set_tacho_stop_action_inx(rsn,TACHO_HOLD);
          //init tacho's speed
          get_tacho_max_speed(lsn, &max_speed);
          if (angle < 0){
            rspeed = (int)((float)(-1)*max_speed * ROTATION_SPEED / 100.0 + 0.5);
          }
          else{
            rspeed = (int)((float)max_speed * ROTATION_SPEED / 100.0 + 0.5);
          }
          lspeed = -rspeed;
          set_tacho_speed_sp( lsn, lspeed );
          set_tacho_speed_sp( rsn, rspeed );
          //init tacho's speed curve shape
          set_tacho_ramp_up_sp( lsn, 50 );
          set_tacho_ramp_up_sp( rsn, 50 );
          set_tacho_ramp_down_sp( lsn, 500 );
          set_tacho_ramp_down_sp( rsn, 500 );
          //init angle start angle
          angle_start = get_angle(gyro_id);
          current_angle = get_angle(gyro_id);
          //launch tacho
          set_tacho_command_inx( lsn, TACHO_RUN_FOREVER );
          set_tacho_command_inx( rsn, TACHO_RUN_FOREVER );
          int inverse = 0;
          while ((abs(abs(angle_start - current_angle) - abs(angle))) > 2){
            //if the robot goes beyond the the asked angle value go back
            if (abs(angle_start - current_angle) - abs(angle) > 0 && inverse == 0){
              rspeed = -rspeed/2;
              lspeed = -lspeed/2;
              inverse = 1;
              set_tacho_speed_sp( lsn, lspeed );
              set_tacho_speed_sp( rsn, rspeed );
              set_tacho_command_inx( lsn, TACHO_RUN_FOREVER );
              set_tacho_command_inx( rsn, TACHO_RUN_FOREVER );
            }
            else if (abs(angle_start - current_angle) - abs(angle) < 0 && inverse == 1){
              rspeed = -rspeed/2;
              lspeed = -lspeed/2;
              inverse = 0;
              set_tacho_speed_sp( lsn, lspeed );
              set_tacho_speed_sp( rsn, rspeed );
              set_tacho_command_inx( lsn, TACHO_RUN_FOREVER );
              set_tacho_command_inx( rsn, TACHO_RUN_FOREVER );
            }
        }
        set_tacho_command_inx( lsn, TACHO_STOP );
        set_tacho_command_inx( rsn, TACHO_STOP );
    }
  }
}

//Nathan
//Make the robot turn based on angle from gyro sensor
//angle to the left is positive angle
void turn_gyro(float angle, uint8_t gyro_id) {

    int range_angle = 2;
    int speed_max = 40;
    int speed_min = 18;

    if (angle == 0) {
        return;
    }
    int angle_start, current_angle;
    uint8_t lsn;
    uint8_t rsn;

    //NOTE : don't use initialisation each time you want to use tacho !!!!
    //config it once and for all !!
    if (ev3_search_tacho_plugged_in(LEFT_WHEEL_PORT, 0, &lsn, 0)) {
        if (ev3_search_tacho_plugged_in(RIGHT_WHEEL_PORT, 0, &rsn, 0)) {

            set_tacho_stop_action_inx(lsn,TACHO_HOLD);
            set_tacho_stop_action_inx(rsn,TACHO_HOLD);

            //init angle start angle
            angle_start = get_angle(gyro_id);
            //printf("angle start = %d \n", angle_start);
            current_angle = angle_start;
            //printf("current angle = %d \n", current_angle);

            //duty_cycle is the roughly the percentage of power given to the tacho
            int duty_cycle = angle - (current_angle - angle_start);

            if (duty_cycle > speed_max || duty_cycle < ((-1) * speed_max) ){
              duty_cycle = duty_cycle / abs(duty_cycle) * speed_max;
            }
            if (duty_cycle > ((-1) * speed_min) && duty_cycle < speed_min ){
              duty_cycle = duty_cycle / abs(duty_cycle) * speed_min;
            }
            //printf("new duty cycle = %d \n", duty_cycle);

            //set the tacho's rotation
            set_tacho_duty_cycle_sp( lsn, duty_cycle );
            set_tacho_duty_cycle_sp( rsn, (-1) * duty_cycle );

            //launch tachos
            set_tacho_command_inx( lsn, TACHO_RUN_DIRECT );
            set_tacho_command_inx( rsn, TACHO_RUN_DIRECT );

            while ((abs(abs(angle_start - current_angle) - angle)) > range_angle){

              //recompute duty cycle value
              duty_cycle = angle - (current_angle - angle_start);
              if (duty_cycle > speed_max || duty_cycle < ((-1) * speed_max) ){
                duty_cycle = duty_cycle / abs(duty_cycle) * speed_max;
              }
              if (duty_cycle > ((-1) * speed_min) && duty_cycle < speed_min ){
                duty_cycle = duty_cycle / abs(duty_cycle) * speed_min;
              }
              //printf("new duty cycle = %d \n", duty_cycle);

              //update duty cycle value
              set_tacho_duty_cycle_sp( lsn, duty_cycle );
              set_tacho_duty_cycle_sp( rsn, (-1) * duty_cycle );
              Sleep(50);
              //update current angle
              current_angle = get_angle(gyro_id);
              // printf("delta angle = %d \n", (current_angle - angle_start));
            }
            set_tacho_command_inx( lsn, TACHO_STOP );
            set_tacho_command_inx( rsn, TACHO_STOP );
        }
    }
}


#ifdef TACHO_DEBUG

/* ********************** MAIN USED FOR TESTS ********************** */
int main(int argc, char *argv[]) {
    uint8_t udsn;
    uint8_t ocsn;
    uint8_t sonar_id, gyro_id;
    int max_speed, speed, rel_pos, distance;

    if (argc != 5) {
        printf("Usage: ./tacho ud_distance oc_distance angle radius\n");
        exit(-1);
    }

    int ud_distance = atoi(argv[1]);
    int oc_distance = atoi(argv[2]);
    float angle     = atof(argv[3]);
    float radius    = atof(argv[4]);
    ev3_sensor_init();
    ev3_search_sensor(LEGO_EV3_GYRO, &gyro_id, 0);
    ev3_search_sensor(LEGO_EV3_US, &sonar_id, 0);

    printf("Up / Down distance: %d\n", ud_distance);
    printf("Open / Close distance: %d\n", oc_distance);
    if (ud_distance > 200 || oc_distance > 200) {
        printf("One of these values seems a little high, continue anyway ? (CTRL + C to quit) ");
        getchar();
    }

    printf("Initializing tachos...\n");
    for (int i = 0; i < 5 && ev3_tacho_init() < 1; i++) Sleep(1000);
    if (ev3_search_tacho_plugged_in(UP_DOWN_TONG_PORT, 0, &udsn, 0)) {
        printf("    Up / Down tacho OK\n");
        if (ev3_search_tacho_plugged_in(OPEN_CLOSE_TONG_PORT, 0, &ocsn, 0)) {
            printf("    Open / Close tacho OK\n");
            set_tacho_stop_action_inx(udsn, TACHO_HOLD);
            set_tacho_stop_action_inx(ocsn, TACHO_HOLD);
            printf("Done.\n");
        } else {
            printf("    Open / Close tacho ERR\n");
            exit(-1);
        }
    } else {
        printf("    Up/ Down tacho ERR\n");
        exit(-1);
    }

    //UP TONGS
    printf("Up / Down tongs... ");
    rel_pos = ud_distance;
    distance = get_avg_distance(sonar_id, NB_SENSOR_MESURE);
    printf("Distance: %d\n", distance);
    if (distance < 40 && ud_distance < 0) {
        printf("Error!\n");
        exit(EXIT_FAILURE);
    }

    if (distance > 40 && ud_distance > 0) {
        printf("Error!\n");
        exit(EXIT_FAILURE);
    }
    get_tacho_max_speed(udsn, &max_speed);
    speed = (int)((float)max_speed * UP_DOWN_SPEED / 100.0 + 0.5);
    set_tacho_speed_sp( udsn, speed );
    set_tacho_ramp_up_sp( udsn, 25 );
    set_tacho_ramp_down_sp( udsn, 100 );
    set_tacho_position_sp( udsn, rel_pos );
    set_tacho_command_inx( udsn, TACHO_RUN_TO_REL_POS );
    wait_tongs(UP_DOWN_ID);
    printf("Done.\n");

    // OPEN TONGS
    printf("Opening / Closing tongs... ");
    rel_pos = oc_distance;
    get_tacho_max_speed(ocsn, &max_speed);
    speed = (int)((float)max_speed * OPEN_CLOSE_SPEED / 100.0 + 0.5);
    set_tacho_speed_sp( ocsn, speed );
    set_tacho_ramp_up_sp( ocsn, 25 );
    set_tacho_ramp_down_sp( ocsn, 100 );
    set_tacho_position_sp( ocsn, rel_pos );
    set_tacho_command_inx( ocsn, TACHO_RUN_TO_REL_POS );
    wait_tongs(OPEN_CLOSE_ID);
    printf("Done.\n");

    printf("Turning left... ");
    Sleep(500);
    if (angle == 0) {
        return 0;
    }
    uint8_t lsn;
    uint8_t rsn;
    int max_speed, speed;
    int count_per_rot;
    float rad = angle/360 * 2*M_PI;
    int rel_pos;

    while (ev3_tacho_init() < 1) Sleep(1000);
    if (ev3_search_tacho_plugged_in(LEFT_WHEEL_PORT, 0, &lsn, 0)) {
        if (ev3_search_tacho_plugged_in(RIGHT_WHEEL_PORT, 0, &rsn, 0)) {
            set_tacho_stop_action_inx(lsn,TACHO_HOLD);
            set_tacho_stop_action_inx(rsn,TACHO_HOLD);
            get_tacho_max_speed(lsn, &max_speed);
            get_tacho_count_per_rot(lsn, &count_per_rot);
            rel_pos = (int)((radius * rad / WHEEL_PERIMETER) * count_per_rot + 0.5);
            speed = (int)((float)max_speed * ROTATION_SPEED / 100.0 + 0.5);
            set_tacho_speed_sp( lsn, speed );
            set_tacho_speed_sp( rsn, speed );
            set_tacho_ramp_up_sp( lsn, 50 );
            set_tacho_ramp_up_sp( rsn, 50 );
            set_tacho_ramp_down_sp( lsn, 50 );
            set_tacho_ramp_down_sp( rsn, 50 );
            set_tacho_position_sp( lsn, -rel_pos );
            set_tacho_position_sp( rsn, rel_pos );
            set_tacho_command_inx( lsn, TACHO_RUN_TO_REL_POS );
            set_tacho_command_inx( rsn, TACHO_RUN_TO_REL_POS );
        }
    }
    wait_tachos();
    printf("Done.");
    ev3_uninit();
}

#endif
