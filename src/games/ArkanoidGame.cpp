#include "games/ArkanoidGame.hpp"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QResizeEvent>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <utility>
namespace games {
ArkanoidGame::ArkanoidGame(QWidget *parent) : GameEngine(parent), m_currentForce(0.5), m_ballRadius(8.0), m_ballSpeed(245.0), m_paddleWidth(130.0), m_paddleHeight(14.0), m_difficultyLevel(1), m_level(1), m_lives(3) { setFocusPolicy(Qt::NoFocus); }
QString ArkanoidGame::gameName() const { return QStringLiteral("Arkanoid Force Trainer"); }
void ArkanoidGame::initialize() {
 m_level=1; m_lives=3; m_currentForce=0.5; m_combo=0; m_comboRemaining=0.0; m_ballTrail.clear(); setDifficultyLevel(m_difficultyLevel);
 const double availableWidth=qMax(40.0,static_cast<double>(width())-20.0);
 m_paddleWidth=qMin(m_paddleWidth,availableWidth);
 m_paddle=QRectF((width()-m_paddleWidth)/2.0,height()-42.0,m_paddleWidth,m_paddleHeight);
 buildLevel(); resetBall();
}
void ArkanoidGame::setDifficultyLevel(int level) {
 m_difficultyLevel=qBound(1,level,5);
 const double scaleX=qMax(0.5,width()/640.0), scaleY=qMax(0.5,height()/480.0);
 m_ballSpeed=205.0+m_difficultyLevel*25.0;
 m_ballRadius=6.0*qMin(scaleX,scaleY);
 m_paddleWidth=qBound(112.0*scaleX,width()*(0.22-0.008*(m_difficultyLevel-1)),160.0*scaleX);
 m_paddleHeight=14.0*scaleY;
 setScore(0);
 if(!m_paddle.isNull()){const double aw=qMax(40.0,static_cast<double>(width())-20.0);m_paddle.setWidth(qMin(m_paddleWidth,aw));m_paddle.moveLeft(qBound(10.0,m_paddle.left(),qMax(10.0,static_cast<double>(width())-10.0-m_paddle.width())));}
 if(isRunning()){m_level=1;m_lives=3;m_combo=0;m_comboRemaining=0.0;buildLevel();resetBall();}
}
int ArkanoidGame::getDifficultyLevel() const{return m_difficultyLevel;}
double ArkanoidGame::paddlePositionX() const{return m_paddle.center().x();}
int ArkanoidGame::comboCount() const{return m_combo;}
void ArkanoidGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial){m_serialPort=std::move(serial);}
void ArkanoidGame::disconnectSerial(){m_serialPort.reset();}
bool ArkanoidGame::isSerialConnected() const{return m_serialPort&&m_serialPort->isConnected();}
bool ArkanoidGame::handleInput(double value){if(!std::isfinite(value))return false;m_currentForce=qBound(MIN_FORCE,value/MAX_FORCE,1.0);return true;}
void ArkanoidGame::resizeEvent(QResizeEvent *event) {
 const QSize oldSize=event->oldSize(), newSize=event->size();
 if(newSize.width()>0&&newSize.height()>0) {
  const double sx=oldSize.width()>0?static_cast<double>(newSize.width())/oldSize.width():newSize.width()/640.0;
  const double sy=oldSize.height()>0?static_cast<double>(newSize.height())/oldSize.height():newSize.height()/480.0;
  if(oldSize.width()>0&&oldSize.height()>0) {
   m_ballPosition=QPointF(m_ballPosition.x()*sx,m_ballPosition.y()*sy);
   m_ballVelocity=QPointF(m_ballVelocity.x()*sx,m_ballVelocity.y()*sy);
   for(QPointF& trailPoint:m_ballTrail)
    trailPoint=QPointF(trailPoint.x()*sx,trailPoint.y()*sy);
   for(Brick& brick:m_bricks)
    brick.rect=QRectF(brick.rect.x()*sx,brick.rect.y()*sy,brick.rect.width()*sx,brick.rect.height()*sy);
  }
  const double scaleX=qMax(0.5,newSize.width()/640.0), scaleY=qMax(0.5,newSize.height()/480.0);
  m_ballRadius=6.0*qMin(scaleX,scaleY);
  m_paddleWidth=qBound(112.0*scaleX,newSize.width()*(0.22-0.008*(m_difficultyLevel-1)),160.0*scaleX);
  m_paddleHeight=14.0*scaleY;
  const double paddleX=qBound(10.0,m_paddle.center().x()*sx,newSize.width()-10.0-m_paddleWidth);
  m_paddle=QRectF(paddleX-m_paddleWidth/2.0,newSize.height()-42.0*scaleY-m_paddleHeight,m_paddleWidth,m_paddleHeight);
  if(oldSize.width()<=0||oldSize.height()<=0) buildLevel();
  m_ballSpeed=205.0+m_difficultyLevel*25.0;
 }
 GameEngine::resizeEvent(event);
}
void ArkanoidGame::buildLevel(){
 m_bricks.clear();if(width()<=0||height()<=120)return;
 const double scaleX=qMax(0.5,width()/640.0),scaleY=qMax(0.5,height()/480.0);
 constexpr int columns=8;const int rows=qBound(4,4+(m_level-1)/2,7);
 const double margin=24.0*scaleX,gap=6.0*scaleX,brickHeight=18.0*scaleY,totalGap=gap*(columns-1);
 const double brickWidth=qMax(12.0*scaleX,(width()-2.0*margin-totalGap)/columns);
 for(int row=0;row<rows;++row)for(int column=0;column<columns;++column){Brick b;b.rect=QRectF(margin+column*(brickWidth+gap),58.0*scaleY+row*(brickHeight+gap),brickWidth,brickHeight);b.row=row;b.hits=(m_level>=3&&row==0)?2:1;b.active=true;m_bricks.push_back(b);}
}
void ArkanoidGame::resetBall(){
 const double scaleX=qMax(0.5,width()/640.0),scaleY=qMax(0.5,height()/480.0);
 m_ballPosition=QPointF(m_paddle.center().x(),m_paddle.top()-m_ballRadius-1.0);
 const double horizontal=m_ballSpeed*0.30*scaleX;
 const double vertical=m_ballSpeed*std::sqrt(1.0-0.30*0.30)*scaleY;
 m_ballVelocity=QPointF(horizontal,-vertical);m_ballTrail.clear();
}
bool ArkanoidGame::hasActiveBricks() const{return std::any_of(m_bricks.cbegin(),m_bricks.cend(),[](const Brick& b){return b.active;});}
void ArkanoidGame::advanceLevel(){++m_level;m_ballSpeed=qMin(m_ballSpeed*1.025,560.0);m_combo=0;m_comboRemaining=0.0;addScore(100);buildLevel();resetBall();}
void ArkanoidGame::update(double dt){
 if(!isRunning()||!std::isfinite(dt)||dt<=0.0||width()<=0||height()<=0)return;
 dt=qMin(dt,0.1);constexpr double maxStep=1.0/120.0;const int steps=qBound(1,static_cast<int>(std::ceil(dt/maxStep)),12);const double step=dt/steps;
 for(int i=0;i<steps&&isRunning();++i)updatePhysicsStep(step);
}
void ArkanoidGame::updatePhysicsStep(double dt){
 if(!isRunning()||width()<=0||height()<=0)return;
 const double maxX=qMax(10.0,static_cast<double>(width())-10.0-m_paddle.width());
 const double sensorX=10.0+m_currentForce*qMax(0.0,static_cast<double>(width())-20.0-m_paddle.width());
 const double alpha=1.0-std::exp(-dt/0.045);
 m_paddle.moveLeft(qBound(10.0,m_paddle.left()+(sensorX-m_paddle.left())*alpha,maxX));
 if(m_comboRemaining>0.0){m_comboRemaining=qMax(0.0,m_comboRemaining-dt);if(m_comboRemaining<=0.0)m_combo=0;}
 const QPointF previous=m_ballPosition;m_ballPosition+=m_ballVelocity*dt;m_ballTrail.push_front(m_ballPosition);while(m_ballTrail.size()>7)m_ballTrail.removeLast();
 if(m_ballPosition.x()-m_ballRadius<0){m_ballPosition.setX(m_ballRadius);m_ballVelocity.setX(std::abs(m_ballVelocity.x()));}
 else if(m_ballPosition.x()+m_ballRadius>width()){m_ballPosition.setX(width()-m_ballRadius);m_ballVelocity.setX(-std::abs(m_ballVelocity.x()));}
 if(m_ballPosition.y()-m_ballRadius<0){m_ballPosition.setY(m_ballRadius);m_ballVelocity.setY(std::abs(m_ballVelocity.y()));}
 if(m_ballPosition.y()-m_ballRadius>height()){--m_lives;m_combo=0;m_comboRemaining=0;if(m_lives<=0){stop();return;}resetBall();return;}
 QRectF bounds(m_ballPosition.x()-m_ballRadius,m_ballPosition.y()-m_ballRadius,2*m_ballRadius,2*m_ballRadius);
 if(m_ballVelocity.y()>0&&previous.y()+m_ballRadius<=m_paddle.top()+2.0&&bounds.intersects(m_paddle)){
  const double half=qMax(1.0,m_paddle.width()/2.0);const double offset=qBound(-1.0,(m_ballPosition.x()-m_paddle.center().x())/half,1.0);
  const double scaleX=qMax(0.5,width()/640.0),scaleY=qMax(0.5,height()/480.0);
  const double logicalSpeed=qMin(std::hypot(m_ballVelocity.x()/scaleX,m_ballVelocity.y()/scaleY)*1.012,820.0);
  const double angle=offset*1.12;
  double vx=std::sin(angle),vy=-qMax(0.38,std::abs(std::cos(angle)));
  const double directionLength=qMax(1.0,std::hypot(vx,vy));
  m_ballVelocity=QPointF(vx/directionLength*logicalSpeed*scaleX,
                         vy/directionLength*logicalSpeed*scaleY);
  m_ballPosition.setY(m_paddle.top()-m_ballRadius-0.5);m_ballTrail.clear();
 }
 for(Brick& b:m_bricks){
  if(!b.active||!bounds.intersects(b.rect))continue;
  --b.hits;++m_combo;m_comboRemaining=2.4;const int multiplier=qBound(1,1+(m_combo-1)/4,5);
  if(b.hits<=0){b.active=false;addScore(10*multiplier);}else addScore(5*multiplier);
  const bool vertical=previous.y()+m_ballRadius<=b.rect.top()||previous.y()-m_ballRadius>=b.rect.bottom();
  if(vertical){if(previous.y()<=b.rect.top())m_ballPosition.setY(b.rect.top()-m_ballRadius-0.1);else m_ballPosition.setY(b.rect.bottom()+m_ballRadius+0.1);m_ballVelocity.setY(-m_ballVelocity.y());}
  else{if(previous.x()<=b.rect.left())m_ballPosition.setX(b.rect.left()-m_ballRadius-0.1);else m_ballPosition.setX(b.rect.right()+m_ballRadius+0.1);m_ballVelocity.setX(-m_ballVelocity.x());}
  if(!hasActiveBricks())advanceLevel();break;
 }
}
QColor ArkanoidGame::brickColor(const Brick& b) const{
 if(b.hits>1)return QColor(245,90,90);switch(b.row%5){case 0:return QColor(245,110,110);case 1:return QColor(245,170,80);case 2:return QColor(235,215,90);case 3:return QColor(100,200,130);default:return QColor(90,160,230);}
}
void ArkanoidGame::render(QPainter& p){
 p.setRenderHint(QPainter::Antialiasing, false);p.fillRect(rect(),QColor(12,18,32));p.setPen(QPen(QColor(35,48,68),1));
 for(int x=20;x<width();x+=40)p.drawLine(x,55,x,height()-28);
 for(const Brick& b:m_bricks){if(!b.active)continue;p.setBrush(brickColor(b));p.setPen(QPen(QColor(20,25,40),1));p.drawRoundedRect(b.rect,3,3);if(b.hits>1){p.setPen(QPen(QColor(255,235,235),1.5));p.drawLine(b.rect.left()+5,b.rect.center().y(),b.rect.right()-5,b.rect.center().y());}}
 p.setPen(Qt::NoPen);p.setBrush(QColor(90,210,235,35));p.drawRoundedRect(m_paddle.adjusted(-4,-3,4,3),8,8);p.setBrush(QColor(90,210,235));p.drawRoundedRect(m_paddle,6,6);
 for(int i=m_ballTrail.size()-1;i>=0;--i){const int alpha=18+(m_ballTrail.size()-i)*17;const double radius=m_ballRadius*(0.35+0.07*(m_ballTrail.size()-i));p.setBrush(QColor(120,220,255,alpha));p.drawEllipse(m_ballTrail.at(i),radius,radius);}
 p.setBrush(Qt::white);p.drawEllipse(m_ballPosition,m_ballRadius,m_ballRadius);p.setPen(Qt::white);p.setFont(QFont(QStringLiteral("Arial"),14,QFont::Bold));
 p.drawText(18,28,QStringLiteral("Wynik: %1").arg(getScore()));p.drawText(18,50,QStringLiteral("Poziom: %1").arg(m_level));p.drawText(width()-130,28,QStringLiteral("Życia: %1").arg(m_lives));
 if(m_combo>=2){p.setPen(QColor(255,220,100));p.drawText(width()/2-65,28,QStringLiteral("SERIA x%1").arg(qBound(1,1+(m_combo-1)/4,5)));}
 p.setPen(QColor(185,205,225));p.setFont(QFont(QStringLiteral("Arial"),9));p.drawText(18,height()-12,QStringLiteral("Tensometr: położenie paletki"));p.drawText(width()-150,height()-12,QStringLiteral("Trudność: %1").arg(m_difficultyLevel));
 if(m_serialPort&&!m_serialPort->isConnected()){p.setPen(Qt::red);p.setFont(QFont(QStringLiteral("Arial"),12,QFont::Bold));p.drawText(rect(),Qt::AlignTop|Qt::AlignHCenter,QStringLiteral("BRAK POŁĄCZENIA Z CZUJNIKIEM"));}
}
} // namespace games
