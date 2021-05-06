 import processing.sound.*;
int delay = 150;
int n_w = 3;
int n_h = 3;
int side;
int delta;
int x,y,x_old,y_old;
SinOsc sine;

float freq[]= {265.625, 312.5, 359.375, 406.25, 453.125, 500, 546.875, 593.75, 640.625};
float start_tune[] = {453.125, 500, 562.5, 453.125, 500, 687.5};
float end_tune[] = {687.5, 453.125, 500, 453.125};
Button resetB;
Button generateB;
Button exitB;
Button upDelayB;
Button downDelayB;

ArrayList<Integer> order = new ArrayList<Integer>();


void setup(){
  fullScreen();  
  side = width/3;
  delta = side/6;
  sine = new SinOsc(this);
  resetB = new Button("CLEAR", 20,n_h*side+delta, 4*70,4*20);
  exitB = new Button("EXIT", 20,n_h*side+delta+4*20+50, 4*70,4*20);
  generateB = new Button("GENERATE", 20+4*70+50,n_h*side+delta, 4*70,4*20);
  upDelayB = new Button("SLOWER", 20,n_h*side+delta+8*20+100, 4*70,4*20);
  downDelayB = new Button("FASTER", 20+4*70+50,n_h*side+delta+8*20+100, 4*70,4*20);
  draw_board();
}

void draw(){
  if (mousePressed == true){
    if (downDelayB.MouseIsOver()) delay-=1;
    if (upDelayB.MouseIsOver()) delay+=1;
    if (exitB.MouseIsOver()) exit();
    if (resetB.MouseIsOver()) order.clear();
    if (generateB.MouseIsOver()) generate();
    int item = get_item();
    if (item != -1){
      if (order.size() == 0){
        order.add(item);
      } else if (order.get(order.size()-1)!=item){
        x = item%n_w;
        y = item/n_w;
        x_old = order.get(order.size()-1)%n_w;
        y_old = order.get(order.size()-1)/n_w;
        int deltaX = abs(x_old-x);
        int deltaY = abs(y_old-y);
        if (deltaX!=deltaY&&deltaX<2&&deltaY<2){
          order.add(item);
          println("x:",x,"y:",y);
        }
      }
    }
  }
  background(200);
  draw_board();
  resetB.Draw();
  generateB.Draw();
  exitB.Draw();
  downDelayB.Draw();
  upDelayB.Draw();
  text("delay: "+delay,width/2,n_h*side+delta+12*20+200);
}

void generate(){
  for (int i = 0; i < start_tune.length; ++i){
    sine.freq(start_tune[i]);
    sine.play();
    delay(delay);
    sine.stop();
    // delay(200);
  }
  for (int i = 0; i < order.size(); ++i){
    sine.freq(freq[order.get(i)]);
    sine.play();
    delay(delay);
    sine.stop();
    // delay(200);
  }
   for (int i = 0; i < end_tune.length; ++i){
    sine.freq(end_tune[i]);
    sine.play();
    delay(delay);
    sine.stop();
    // delay(200);
  }

}


class Button{
  String label;
  float x,y,w,h;

  Button(String labelB, float xpos, float ypos, float widthB, float heightB){
    label = labelB;
    x = xpos;
    y = ypos;
    w = widthB;
    h = heightB;
  }

  void Draw(){
    fill(255);
    stroke(141);
    rect(x,y,w,h,10);
    textSize(26); 
    textAlign(CENTER,CENTER);
    fill(0);
    text(label, x+ w/2, y + h/2);
  }

  boolean MouseIsOver(){
    if (mouseX > x && mouseX < x+w && mouseY > y && mouseY < y+h){
      return true;
    }
    return false;
  }
}

void draw_title(){
    fill(255);
    stroke(141);
    int x = 0;
    int y = height-100;
    int w = width;
    int h = 100;
    rect(x,y,w,h,20);
    textSize(40); 
    textAlign(CENTER,CENTER);
    fill(0);
    text("PATH GENERATOR", x+ w/2, y + h/2);
}

void draw_board(){
  fill(255);
  stroke(0);
  draw_title();
  fill(255);
  stroke(0);
  for(int i = 0; i < n_h; ++i){
    for (int j = 0; j < n_w; ++j){
      rect(i*side,j*side,side,side);
    }
  }
  fill(0);
  if (order.size()>0){
    x = order.get(0)%n_w;
    y = order.get(0)/n_w;
    rect(x*side+delta,y*side+delta, side - 2*delta, side - 2*delta);
  }

  for(int i = 1; i < order.size(); ++i){
    x = order.get(i)%n_w;
    y = order.get(i)/n_w;
    x_old = order.get(i-1)%n_w;
    y_old = order.get(i-1)/n_w;
    rect(x*side+delta,y*side+delta, side - 2*delta, side - 2*delta);
    int deltaX = (x_old-x);
    int deltaY = (y_old-y);
    if (deltaX==1){
      deltaX = side;
      deltaY = side-2*delta;
    } else if (deltaX == -1){
      deltaX = -2*delta;
      deltaY = side-2*delta;
    } else if (deltaY == 1){
      deltaY = side;
      deltaX = side-2*delta;
    } else if (deltaY == -1){
      deltaY = -2*delta;
      deltaX = side-2*delta;
    }
    rect(x*side+delta,y*side+delta, deltaX, deltaY);
  }

  fill(255);
}

int get_item(){
  int item_x = (int) mouseX / side;
  int item_y = (int) mouseY / side;
  int item = item_x+item_y*n_w;
  if (item_x>= n_h || item_y >=  n_w){
    item = -1;
  }
  return item;
}
