#include "A50_Cellmask.h"

class Automaton {

  
  private:
    int _target;
    int _reward, _penalty;
    int _floorInit, _minInit, _maxInit;
    boolean _isBg;
    
  
  public: 
    int cells[SIZE_SCREEN][SIZE_SCREEN]; 
    int nextCells[SIZE_SCREEN][SIZE_SCREEN];
    Cellmask (*_cellmask)[SIZE_SCREEN];
    const boolean (*_background)[SIZE_SCREEN];
    Automaton(int target, int reward, int penalty, int floorInit, int minInit, int maxInit, bool isBg, Cellmask current_mask[SIZE_SCREEN][SIZE_SCREEN], const boolean current_bg[SIZE_SCREEN][SIZE_SCREEN]) {
      _target = target;
      _penalty = penalty;
      _reward = reward;
      _floorInit = floorInit;
      _minInit = minInit;
      _maxInit = maxInit;
      _isBg = isBg;
      _cellmask = current_mask;
      _background = current_bg;
    }


    
    void init(){
      for (int x = 0; x < SIZE_SCREEN; x++){
        for (int y = 0; y < SIZE_SCREEN; y++){
          if (can_color(x, y)){
            int hue_val = _cellmask[x][y].getHue();
            cells[x][y] = hue_val - int(random(_maxInit - _minInit + 2) + _minInit - 1);   
          }
        }
      }
    }
  
    void iterate(){
      int neighbours[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      for (int x = 0; x < SIZE_SCREEN; x++){
        for (int y = 0; y < SIZE_SCREEN; y++){
          int index = 0;
          if (can_color(x, y)) {
             for (int dx = -1; dx <= 1; dx++){
              for (int dy = -1; dy <= 1; dy++){ 
                neighbours[index] = cells[mod(x+dx, SIZE_SCREEN)][mod(y+dy, SIZE_SCREEN)]; //_neighbourMask.charAt(index) == '0' ? INT_MIN : cells_A[mod(x+dx, SIZE_SCREEN_A)][mod(y+dy, SIZE_SCREEN_A)];
                index++;
              }
            }
          
          boolean condition = canSum(neighbours, sizeof(neighbours)/sizeof(neighbours[0]), _target + cells[x][y]);
          if (condition){
            nextCells[x][y] = (cells[x][y] + _reward);
          } else {
            nextCells[x][y] = (cells[x][y] - _penalty);
          }
          if (nextCells[x][y] < _floorInit){
            nextCells[x][y] = _floorInit;
          }
          }
        }
      }
      for (int x = 0; x < SIZE_SCREEN; x++){
        for (int y = 0; y < SIZE_SCREEN; y++){
          cells[x][y] = nextCells[x][y];
        }
      }
    }

    void iterate_growth(){
      int neighbours[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      for (int x = 0; x < SIZE_SCREEN; x++){
        for (int y = 0; y < SIZE_SCREEN; y++){
          int index = 0;
          for (int dx = -1; dx <= 1; dx++){
            for (int dy = -1; dy <= 1; dy++){ 
              neighbours[index] = cells[mod(x+dx, SIZE_SCREEN)][mod(y+dy, SIZE_SCREEN)]; //_neighbourMask.charAt(index) == '0' ? INT_MIN : cells_A[mod(x+dx, SIZE_SCREEN_A)][mod(y+dy, SIZE_SCREEN_A)];
              index++;
            }
          }
          boolean condition = canSum(neighbours, sizeof(neighbours)/sizeof(neighbours[0]), _target + cells[x][y]);
          if (condition){
            nextCells[x][y] = (cells[x][y] + _reward);
          } else {
            nextCells[x][y] = (cells[x][y] - _penalty);
          }
          if (nextCells[x][y] < _floorInit){
            nextCells[x][y] = _floorInit;
          }
        }
      }
      for (int x = 0; x < SIZE_SCREEN; x++){
        for (int y = 0; y < SIZE_SCREEN; y++){
          cells[x][y] = nextCells[x][y];
        }
      }
      }

    int getSums(int neighbours[], int neighboursLength, int target){
      int canSum = 0;
      for (int i = 1; i < pow(2, neighboursLength); i++){
        int sum = 0;
        for (int j = 0; j < neighboursLength; j++){
          if ((i >> j) % 2 == 1){
            sum += neighbours[j];
            
          }
        }
        if (sum == target) {
          canSum ++;
          return canSum;
        }
      }
      return canSum;
    }
  
    boolean canSum(int neighbours[], int neighboursLength, int target){
      if (getSums(neighbours, neighboursLength, target) > 0){
        return true;
      } else {
        return false;
      }
    }
  
  int mod(int x, int m){
    return ((x % m + m) % m);
  }

  int colorMap(int x, int y){
    //float val = value * 0.001; ///rework this
    //int b = (val >= 1.0 ? 255 : (val <= 0.0 ? 0 : (int)floor(val * 256.0)));
    return cells[x][y] % 360;
  }

  int brightnessMap(int x, int y){
    //float val = value * 0.001; ///rework this
    //int b = (val >= 1.0 ? 255 : (val <= 0.0 ? 0 : (int)floor(val * 256.0)));
    return (cells[x][y] * 20) % 255;
  }

  int saturationMap(int x, int y){
    //float val = value * 0.001; ///rework this
    //int b = (val >= 1.0 ? 255 : (val <= 0.0 ? 0 : (int)floor(val * 256.0)));
    return max((cells[x][y] + 150) % 255, 100);
  }
  boolean allZero() {
    for (int x = 0; x < SIZE_SCREEN; x++) {
      for (int y = 0; y < SIZE_SCREEN; y++) {
        if (cells[x][y] != 0) {
          return false;
        }
      }
    }
    return true;
  }

  boolean can_color(int x, int y){
    if (_isBg){
      return _background[x][y];
    } else {
      return !_background[x][y];
    }
  }

  void init_line() {
    int maxInit = 16;
    int minInit = 8;
    int cx, cy;
    cx = int(random(maxInit - minInit + 2) + minInit - 1);
    cy = int(random(maxInit - minInit + 2) + minInit - 1);
    for (int y = cy - 3 ; y < cy + 3; y++){
      cells[cx][y] = int(random(_maxInit - _minInit + 2) + _minInit - 1);   
    }
  }

  void init_square(){
    int maxInit = 16;
    int minInit = 8;
    int cx, cy;
    cx = int(random(maxInit - minInit + 2) + minInit - 1);
    cy = int(random(maxInit - minInit + 2) + minInit - 1);
    for (int y = cy - 3 ; y < cy + 3; y++){
      for (int x = cx - 3; x < cx + 3; x++){
        cells[x][y] = int(random(_maxInit - _minInit + 2) + _minInit - 1);   
      }
    }
  }

  void init_multiple(int num){
    int maxInit = 18;
    int minInit = 6;
    int cx, cy;
    for(int k = 0; k < num; k++){
      cx = int(random(maxInit - minInit + 2) + minInit - 1);
      cy = int(random(maxInit - minInit + 2) + minInit - 1);
      for (int y = cy; y < cy + 1; y++){
        for (int x = cx; x < cx + 1; x++){
          cells[x][y] = int(random(_maxInit - _minInit + 2) + _minInit - 1);   
        }
      }
    }

  }

};