#include "Path.h"
#include "Error.h"
//����
Point2D catmull_rom(PointGroup pg,float t) {
    float t2 = t * t;
    float t3 = t2 * t;

    Point2D* p0=pg.p0;
    Point2D* p1=pg.p1;
    Point2D* p2=pg.p2;
    Point2D* p3=pg.p3;

    Point2D result;


    // ������ϵ��
    float a0 = -0.5f * t3 + t2 - 0.5f * t;
    float a1 = 1.5f * t3 - 2.5f * t2 + 1.0f;
    float a2 = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
    float a3 = 0.5f * t3 - 0.5f * t2;

    result.x = a0 * p0->x + a1 * p1->x + a2 * p2->x + a3 * p3->x;
    result.y = a0 * p0->y + a1 * p1->y + a2 * p2->y + a3 * p3->y;
    return result;
}
//һ�׵�
Point2D catmull_rom_derivative(PointGroup pg, float t) {
  float t2 = t * t;

  Point2D* p0=pg.p0;
  Point2D* p1=pg.p1;
  Point2D* p2=pg.p2;
  Point2D* p3=pg.p3;

  Point2D result;

  // ���㵼��ϵ��
  float da0 = -1.5f * t2 + 2.0f * t - 0.5f;
  float da1 = 4.5f * t2 - 5.0f * t;
  float da2 = -4.5f * t2 + 4.0f * t + 0.5f;
  float da3 = 1.5f * t2 - 1.0f * t;

  result.x= da0 * p0->x + da1 * p1->x + da2 * p2->x + da3 * p3->x;
  result.y=da0 * p0->y + da1 * p1->y + da2 * p2->y + da3 * p3->y;
    return result;
  }
//���׵�
Point2D catmull_rom_second_derivative(PointGroup pg, float t) {
    Point2D* p0=pg.p0;
    Point2D* p1=pg.p1;
    Point2D *p2=pg.p2;
    Point2D* p3=pg.p3;
    // ������׵���ϵ��
    float dda0 = -3.0f * t + 2.0f;
    float dda1 = 9.0f * t - 5.0f;
    float dda2 = -9.0f * t + 4.0f;
    float dda3 = 3.0f * t - 1.0f;

    Point2D result;


  
    result.x = dda0 * p0->x + dda1 * p1->x + dda2 * p2->x + dda3 * p3->x;
    result.y = dda0 * p0->y + dda1 * p1->y + dda2 * p2->y + dda3 * p3->y;
    return result;
}
PointGroup select_group(Point2D *list,float t){
    //TODO: t��С�Ĵ�����
    PointGroup pg;
    int start=(int)t;
    pg.p0=list+start;
    pg.p1=list+start+1;
    pg.p2=list+start+2;
    pg.p3=list+start+3;

    return pg;
}
Point2D *list;      //��ǰѡ�е�·��
Path* selected_path=0;
Point2D curve( float t) {
    return catmull_rom(select_group(list,t),t-(float)((int)t));
}
Point2D dcurve(float t){
    return catmull_rom_derivative(select_group(list,t),t-(float)((int)t));
}
Point2D ddcurve( float t) {
    return catmull_rom_second_derivative(select_group(list,t),t-(float)((int)t));
}
//ѡ��pathȥ����
void Path_Select(Path* path){
    selected_path=path;
}
float Path_GetDirection(){
    if(selected_path==0){
        ERROR(4,"ldp");
        return 0.0;
    }
    //TODO:Finish this
}
float Path_GetNormalError(){
    if(selected_path==0){
        ERROR(4,"ldp");
        return 0.0;
    }
    //TODO:��ʹ�����ߵ�ʱ��������
    return 0.0;
}


// ref_position �ο�λ��
// start_point ��ʼ�ƽ������
// step ����
// precise ��ȷ��Ҫ�󣨱���Ҫ���������С���������
Point2D Find_Nearest_Point_In_Path(Point2D* list,Point2D* ref_position,float start_point,float step,float precise,const float epoch){
    float previous_t=0.0;
    float now_t=start_point;
    int i=0;
    Point2D now_position=curve(now_t);
    Point2D now_position_d=dcurve(now_t);
    Point2D now_position_dd=ddcurve(now_t);
    float dot=Vec_Dot(Vec_Sub(now_position,*ref_position),now_position_d);      //������������������
    float ddot=Vec_Dot(Vec_Sub(now_position_d,now_position_dd),now_position_d)+Vec_Dot(now_position_d,now_position_d);      //�������ĵ��� ָʾ�½�����
    float previous_dot=-114514.0f;
    
    for(i=0;i<epoch;i++){
        now_t+=signf(dot)*signf(ddot)*step*(-1);    //����t
        
        //��������
        now_position=curve(now_t);
        now_position_d=dcurve(now_t);
        now_position_dd=ddcurve(now_t);
        dot=Vec_Dot(Vec_Sub(now_position,*ref_position),now_position_d);      
        ddot=Vec_Dot(Vec_Sub(now_position_d,now_position_dd),now_position_d)+Vec_Dot(now_position_d,now_position_d);
        
        if (fabs(ddot) < precise) {
            goto success;
        }

        if (previous_dot!=-114514.0f && dot * previous_dot < 0) {
            //˵������0��
            goto found;
        }
        previous_dot = dot;
        previous_t = now_t;
    }
    //epoch������ 
    goto error;


    //����ҵ�
    found:
        //���ַ� ����
    {
        float a=previous_t>now_t?now_t:previous_t;
        float b=previous_t>now_t?previous_t:now_t;
        for (; i < epoch; i++) {
            float t=(a+b)/2;
            //��龫���Ƿ�ﵽ
            if(Vec_Dot(Vec_Sub(curve(t),*ref_position),dcurve(t))<precise){
                now_t=t;
                goto success;
            }
            //�Ǽ�������
            if(Vec_Dot(Vec_Sub(curve(a),*ref_position),ddcurve(a))*Vec_Dot(Vec_Sub(curve(t),*ref_position),dcurve(t))<0){
                //�������
                b=t;
            }else{
                a=t;
            }
        }
    }

    goto error;

    success:
    //���������Ѿ����� ֱ�ӷ���
    return curve(now_t);

    

    error:
    //FIXME:���δ���� ���Ʋ���Ҳû��ϵ ����Ҫ���㴦��
    return curve(now_t);
}