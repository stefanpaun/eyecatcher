
#include "Arduino.h"

#define RGB_MODE 0
#define HSB_MODE 1

// ---- color stuff ----

struct RGB {
  byte r;  // 255
  byte g;  // 255
  byte b;  // 255
};

struct HSB {
  int h;  // 360
  byte s;  // 100
  byte b;  // 100
};

class Color {
	private:
		RGB RGB_color;
		HSB HSB_color;
		boolean RGB_updated;
		boolean HSB_updated;

	public:
		

Color(){
	RGB_updated = false;
	HSB_updated = false;
}

Color(int v1, byte v2, byte v3, byte mode){
	if(mode == HSB_MODE){
		setHSB(v1, v2, v3);
		return;
	}
	setRGB(v1, v2, v3);
}

void setRGB(byte r, byte g, byte b){
	RGB_color.r = r;
	RGB_color.g = g;
	RGB_color.b = b;
	RGB_updated = true;
	HSB_updated = false;
}

void setHSB(unsigned int h, byte s, byte b){
	HSB_color.h = (h+720)%360;
	HSB_color.s = s;
	HSB_color.b = b;
	RGB_updated = false;
	HSB_updated = true;
}

void updateRGB(){
	RGB_color = HSBtoRGB(&HSB_color);
	RGB_updated = true;
}

void updateHSB(){
	HSB_color = RGBtoHSB(&RGB_color);
	HSB_updated = true;
}

byte red(){
	if(!RGB_updated){
		updateRGB();
	}
	return RGB_color.r;
}

byte green(){
	if(!RGB_updated){
		updateRGB();
	}
	return RGB_color.g;
}

byte blue(){
	if(!RGB_updated){
		updateRGB();
	}
	return RGB_color.b;
}

int hue(){
	if(!HSB_updated){
		updateHSB();
	}
	return HSB_color.h;
}

byte saturation(){
	if(!HSB_updated){
		updateHSB();
	}
	return HSB_color.s;
}

byte brightness(){
	if(!HSB_updated){
		updateHSB();
	}
	return HSB_color.b;
}

void add(Color toAdd, float fadeFac){
	if(fadeFac!=0.){
		byte r = constrain(red() + toAdd.red()*fadeFac, 0,255);
		byte g = constrain(green() + toAdd.green()*fadeFac, 0,255);
		byte b = constrain(blue() + toAdd.blue()*fadeFac, 0,255);
		
		setRGB(r,g,b);
	}
}

void add(Color toAdd){
	byte r = constrain(red() + toAdd.red(), 0,255);
	byte g = constrain(green() + toAdd.green(), 0,255);
	byte b = constrain(blue() + toAdd.blue(), 0,255);
	setRGB(r,g,b);
}

void addHDR(Color toAdd, float fadeFac){
	if(fadeFac!=0.){
		int r = red() + toAdd.red()*fadeFac;
		int g = green() + toAdd.green()*fadeFac;
		int b = blue() + toAdd.blue()*fadeFac;
		
		int maxVal = r;
		if( g > maxVal ) maxVal = g;
		if( b > maxVal ) maxVal = b;
		
		if( maxVal > 255){
			float mult = 255./maxVal;
			r *= mult;
			g *= mult;
			b *= mult;
		}
		
		setRGB(r,g,b);
	}
}

void multiply(Color toMult, float fadeFac){
	if(fadeFac!=0.){
		byte r = red();
		byte g = green();
		byte b = blue();
		
		if(toMult.red()!=255) r = r * (toMult.red()+(255-toMult.red())*(1-fadeFac)) / 255;
		if(toMult.green()!=255) g = g * (toMult.green()+(255-toMult.green())*(1-fadeFac)) / 255;
		if(toMult.blue()!=255) b = b * (toMult.blue()+(255-toMult.blue())*(1-fadeFac)) / 255;
		
		setRGB(r,g,b);
	}
}

void multiply(Color toMult){
	byte r = red();
	byte g = green();
	byte b = blue();
	
	if(toMult.red()!=255) r = r * toMult.red() / 255.;
	if(toMult.green()!=255) g = g * toMult.green() / 255.;
	if(toMult.blue()!=255) b = b * toMult.blue() / 255.;
	
	setRGB(r,g,b);
}

void multiply(float multFac){
	if(multFac!=1.){
		byte r = red()*multFac;
		byte g = green()*multFac;
		byte b = blue()*multFac;
		setRGB(r,g,b);
	}
}

void fade(Color toFade, float fadeFac){
	if(fadeFac==1.){
		setRGB(toFade.red(),toFade.green(),toFade.blue());
	}else if(fadeFac!=0.){
		byte r = constrain(red()*(1-fadeFac) + toFade.red()*fadeFac, 0,255);
		byte g = constrain(green()*(1-fadeFac) + toFade.green()*fadeFac, 0,255);
		byte b = constrain(blue()*(1-fadeFac) + toFade.blue()*fadeFac, 0,255);
		
		setRGB(r,g,b);
	}
}

void fade(Color c1, Color c2, float fadeFac){
	setRGB(c1.red(),c1.green(),c1.blue());
	fade(c2, fadeFac);
}

//// convert hsb to rgb
RGB HSBtoRGB(HSB *color){
  RGB output;
  int hue = color->h%360;
  byte bri = min(color->b,100)*2.55;
  if (color->s == 0) { // Acromatic color (gray). Hue doesn't mind.
    output.r=bri;
    output.g=bri;
    output.b=bri;  
  }
  else{
    byte base = ((255 - int(min(color->s,100)*2.55)) * bri)>>8;

    switch(hue/60) {
    case 0:
      output.r = bri;
      output.g = (((bri-base)*hue)/60)+base;
      output.b = base;
      break;

    case 1:
      output.r = (((bri-base)*(60-(hue%60)))/60)+base;
      output.g = bri;
      output.b = base;
      break;

    case 2:
      output.r = base;
      output.g = bri;
      output.b = (((bri-base)*(hue%60))/60)+base;
      break;

    case 3:
      output.r = base;
      output.g = (((bri-base)*(60-(hue%60)))/60)+base;
      output.b = bri;
      break;

    case 4:
      output.r = (((bri-base)*(hue%60))/60)+base;
      output.g = base;
      output.b = bri;
      break;

    case 5:
      output.r = bri;
      output.g = base;
      output.b = (((bri-base)*(60-(hue%60)))/60)+base;
      break;
    }
  }
  return output;
}

HSB RGBtoHSB(RGB *color){
  byte max_rgb = max(max(color->r,color->g), color->b);
  byte min_rgb = min(min(color->r,color->g), color->b);
  
  HSB output;
  output.s = max_rgb - min_rgb;

  if (output.s == 0) output.h = 0;
  else if (max_rgb == color->r){
    if(color->b>color->g) output.h = 360+(0 + (float)(color->g-color->b) /  output.s) * 60;
    else output.h = (0 + (float)(color->g-color->b) /  output.s) * 60;
  } 
  else if (max_rgb == color->g) output.h = (2 + (float)(color->b-color->r) /  output.s) * 60;
  else if (max_rgb == color->b) output.h = (4 + (float)(color->r-color->g) /  output.s) * 60;

  output.s = (float)output.s/max_rgb*100;
  output.b = max_rgb/2.55;

  return output;
}

};

