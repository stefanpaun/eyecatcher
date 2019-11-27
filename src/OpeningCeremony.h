// #include "Arduino.h"

// class PatternSpiro {
//   private:
//     byte theta1 = 0;
//     byte theta2 = 0;
//     byte hueoffset = 0;

//     uint8_t radiusx = SIZE_SCREEN / 4;
//     uint8_t radiusy = SIZE_SCREEN / 4;
//     uint8_t minx = MATRIX_CENTER_X - radiusx;
//     uint8_t maxx = MATRIX_CENTER_X + radiusx + 1;
//     uint8_t miny = MATRIX_CENTER_Y - radiusy;
//     uint8_t maxy = MATRIX_CENTER_Y + radiusy + 1;

//     uint8_t spirocount = 1;
//     uint8_t spirooffset = 256 / spirocount;
//     boolean spiroincrement = false;

//     boolean handledChange = false;

//   public:
//     PatternSpiro() {
//       name = (char *)"Spiro";
//     }

//     unsigned int drawFrame() {
//       effects.DimAll(254);

//       boolean change = false;
      
//       for (int i = 0; i < spirocount; i++) {
//         uint8_t x = mapsin8(theta1 + i * spirooffset, minx, maxx);
//         uint8_t y = mapcos8(theta1 + i * spirooffset, miny, maxy);

//         uint8_t x2 = mapsin8(theta2 + i * spirooffset, x - radiusx, x + radiusx);
//         uint8_t y2 = mapcos8(theta2 + i * spirooffset, y - radiusy, y + radiusy);

//         CRGB color = effects.ColorFromCurrentPalette(hueoffset + i * spirooffset, 128);
//         effects.leds[XY(x2, y2)] += color;
        
//         if((x2 == MATRIX_CENTER_X && y2 == MATRIX_CENTER_Y) ||
//            (x2 == MATRIX_CENTRE_X && y2 == MATRIX_CENTRE_Y)) change = true;
//       }

//       theta2 += 1;

//       EVERY_N_MILLIS(25) {
//         theta1 += 1;
//       }

//       EVERY_N_MILLIS(100) {
//         if (change && !handledChange) {
//           handledChange = true;
          
//           if (spirocount >= MATRIX_WIDTH || spirocount == 1) spiroincrement = !spiroincrement;

//           if (spiroincrement) {
//             if(spirocount >= 4)
//               spirocount *= 2;
//             else
//               spirocount += 1;
//           }
//           else {
//             if(spirocount > 4)
//               spirocount /= 2;
//             else
//               spirocount -= 1;
//           }

//           spirooffset = 256 / spirocount;
//         }
        
//         if(!change) handledChange = false;
//       }

//       EVERY_N_MILLIS(33) {
//         hueoffset += 1;
//       }

//       return 0;
//     }
// };