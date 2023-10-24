#include <NintendoExtensionCtrl.h>
#include <USB-MIDI.h>
#include <math.h>
USBMIDI_CREATE_DEFAULT_INSTANCE();

Nunchuk nchuk;

//連打対策のため変数の数でMIDIのOn Offを切り替える
long scale = 0;
int pich = 0;
int zButtonPressedLastCycle = 0;
int cButtonPressedLastCycle = 0;
int def = 60;
int previous_scale = 0;

void setup() {
  Serial.begin(115200);
  nchuk.begin();
  MIDI.begin(MIDI_CHANNEL_OFF);


  while (!nchuk.connect()) {
    Serial.println("Nunchuk not detected!");
    delay(1000);
  }
}

void loop() {
  Serial.println("----- Nunchuk Demo -----");  // Making things easier to read

  boolean success = nchuk.update();  // Get new data from the controller

  if (!success) {  // Ruh roh
    Serial.println("Controller disconnected!");
    delay(1000);
  } else {
    // Read a button (on/off, C and Z)
    boolean zButton = nchuk.buttonZ();
    boolean cButton = nchuk.buttonC();

    //＋１２

    Serial.print("The Z button is ");
    if (zButtonPressedLastCycle == 0 && zButton == true) {
      Serial.println("pressed");
      pich -= 12;
      zButtonPressedLastCycle = 1;
    } else if (zButton == false) {
      Serial.println("released");
      zButtonPressedLastCycle = 0;
    }

    Serial.print("The c button is ");
    if (cButtonPressedLastCycle == 0 && cButton == true) {
      Serial.println("pressed");
      pich += 12;
      cButtonPressedLastCycle = 1;
    } else if (cButton == false) {
      Serial.println("released");
      cButtonPressedLastCycle = 0;
    }


    int joyY = nchuk.joyY();
    int joyX = nchuk.joyX();

    //Serial.print("The joystick's X-axis is at ");
    //Serial.println(joyX);
    //Serial.print("The joystick's Y axis is at ");
    //Serial.println(joyY);

    //原点との差をとってif文書く
    double diff_X = joyX - 127;
    double diff_Y = joyY - 127;
    Serial.print("The diff_X is at ");
    Serial.println(diff_X);
    Serial.print("The diff_Y is at ");
    Serial.println(diff_Y);

    //tan(y/x)
    double tan = atan2(diff_X, diff_Y);
    Serial.print("The tanjent is at ");
    Serial.println(tan);

    int r = diff_X * diff_X + diff_Y * diff_Y;
    Serial.print("The range is at ");
    Serial.println(r);


    //delay(100);

    //Initial position
    /* x = 123 , y = 133 , ratio = 1.08
       max_x =223 , max_y =227
       min_x = 26 , min_y = 32
       upper_right = (196,205) , upper_left = (52,202)
       lower_right = (196,57) , lower_ left = (55,57)
    */
    //range_x = 250 (-125 -> 125) ,range_y = 250 (-125 -> 125)

    int rr = 6000;

    //joystick
    if(r <= rr){
      scale = -1;
    }else{
      scale = stick_radian_scale(diff_X,diff_Y);
    }


    int now_scale = def + scale + pich;

    //もし演奏時に違和感を感じたら、iをそれぞれ音に合わせてint d,r,m,f,...とすると良い
    if (scale == -1) {
      MIDI.sendControlChange(midi::AllNotesOff, 0, 1);
    } else if (now_scale != previous_scale) {
      MIDI.sendNoteOff(previous_scale, 127, 1);
      MIDI.sendNoteOn(now_scale, 127, 1);  //ド
    }

    previous_scale = now_scale;

    //accel 511
    //-8192 ~ 8191

    // Read an accelerometer and print values (0-1023, X, Y, and Z)
    int accelZ = nchuk.accelZ();
    int adjust_accelZ = (double)(accelZ - 511) / 1024 * 16384;

    //Serial.print("The accelerometer's X-axis is at ");
    Serial.println(accelZ);

    MIDI.sendPitchBend(adjust_accelZ, 1);
    //Serial.println(scale_memory);
    //delay(1000);
    // Print all the values!
    // nchuk.printDebug();
  }
}

double stick_radian_scale(double diff_X, double diff_Y) {
  //tan(y/x)
    double tan = atan2(diff_X, diff_Y);
    Serial.print("The tanjent is at ");
    Serial.println(tan);
  //joystick
  int scale;
  if ((diff_X >= 0 && diff_Y >= 0 && tan <= 0.4014) || (diff_X <= 0 && diff_Y >= 0 && tan > -0.4014)) {
    scale = 0;  //328-23 ド
  } else if (diff_X >= 0 && diff_Y >= 0 && tan <= 1.1693 && tan > 0.4014) {
    scale = 2;  //23-67　レ
  } else if ((diff_X >= 0 && diff_Y >= 0 && tan > 1.1693) || (diff_X >= 0 && diff_Y <= 0 && tan <= 1.9547)) {
    scale = 4;  //67-112　ミ
  } else if (diff_X >= 0 && diff_Y <= 0 && tan > 1.9547 && tan <= 2.7401) {
    scale = 5;  //112-157　ファ
  } else if ((diff_X >= 0 && diff_Y <= 0 && tan > 2.7401) || (diff_X <= 0 && diff_Y <= 0 && tan <= -2.7576)) {
    scale = 7;  //157-202　ソ
  } else if (diff_X <= 0 && diff_Y <= 0 && tan > -2.7576 && tan <= -1.9722) {
    scale = 9;  //202-247　ラ
  } else if ((diff_X <= 0 && diff_Y <= 0 && tan > -1.9722) || (diff_X <= 0 && diff_Y >= 0 && tan <= -1.1868)) {
    scale = 11;
  } else if (diff_X <= 0 && diff_Y >= 0 && tan > -1.1868 && tan <= -0.4014) {
    scale = 12;
  } else {
    scale = -1;
  }
  return scale;
}