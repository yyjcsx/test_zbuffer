#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include<iostream>
#include<memory>

const int POINTNUM = 3;

//����AET��ET�Ļ����ṹ
typedef struct XET {
	float x;
	float dx, ymax;
	XET* next;

}AET, ET;





void draw_color(std::vector<Vec3f>& model_face, TGAImage& image) {
	auto this_color = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);



	//ɨ��һ�¼���ymin��ymax
	int Y_max = 0;
	int Y_min = 99999;
	int i;
	for (i = 0; i < POINTNUM; i++) {
		if (model_face[i].y > Y_max) {
			Y_max = model_face[i].y;
		}
		if (model_face[i].y < Y_min) {
			Y_min = model_face[i].y;
		}
	}
	//���л�߱�ĳ�ʼ��
	AET* pAET = new AET;
	pAET->next = NULL;
	//���б߱�ĳ�ʼ��
	ET* pET[2048];//�������PPT�������ŵ���һ��
	for (i = Y_min; i <= Y_max; ++i) {
		pET[i] = new ET;
		pET[i]->next = NULL;
		//pET[i] = NULL;
	}

	//ɨ�貢�����߱�
	std::vector<ET> ET_bridge;
	for (i = Y_min; i <= Y_max; ++i) {
		for (int j = 0; j < POINTNUM; ++j) {
			ET* bridge = NULL;

			if (model_face[j].y == i) {

				if (model_face[(j - 1 + POINTNUM) % POINTNUM].y > model_face[j].y)
				{
					ET* p = new ET;
					p->x = model_face[j].x;
					p->ymax = model_face[(j - 1 + POINTNUM) % POINTNUM].y;
					p->dx = ((model_face[(j - 1 + POINTNUM) % POINTNUM].x - model_face[j].x) / (model_face[(j - 1 + POINTNUM) % POINTNUM].y - model_face[j].y));

					//p->next = pET[i]->next;
					//pET[i]->next = p->next;



					bridge = pET[i]->next;
					pET[i]->next = p;
					p->next = bridge;


				}
				//std::cout << i<<std::endl;
				//std::cout << pET[i]->next << std::endl;

				if (model_face[(j + 1 + POINTNUM) % POINTNUM].y > model_face[j].y)
				{
					ET* p = new ET;
					p->x = model_face[j].x;
					p->ymax = model_face[(j + 1 + POINTNUM) % POINTNUM].y;
					p->dx = ((model_face[(j + 1 + POINTNUM) % POINTNUM].x - model_face[j].x) / (model_face[(j + 1 + POINTNUM) % POINTNUM].y - model_face[j].y));
					//p->next = pET[i]->next;
					//pET[i]->next = p->next;
					bridge = pET[i]->next;
					pET[i]->next = p;
					p->next = bridge;

				}

			}

		}
	}


	for (i = Y_min; i <= Y_max; i++) {


		//�ȸ���AET�е����ݣ��յĻ��Ͳ��ù�ֱ��NULL�ˣ���Ҫ������е�һ��ʱ��ķǿ����
		ET* p = pAET->next;
		while (p != NULL) {
			p->x = p->x + p->dx;
			p = p->next;
		}


		//AET����
		AET* tq = pAET;
		p = pAET->next;
		tq->next = NULL;
		while (p != NULL) {

			while (tq->next != NULL && p->x >= tq->next->x) {
				{tq = tq->next; }
			}
			ET* s = p->next;
			p->next = tq->next;
			tq->next = p;
			p = s;
			tq = pAET;
		}


		AET* q = pAET;
		p = q->next;
		while (p != NULL) {
			if (p->ymax == i) {
				q->next = p->next;
				delete p;
				p = q->next;
			}
			else {
				q = q->next;
				p = q->next;
			}
		}
		///////

		p = pET[i]->next;
		//std::cout << p->x << std::endl;
		q = pAET;
		while (p != NULL)
		{
			while (q->next != NULL && p->x >= q->next->x)
			{
				q = q->next;
			}

			ET* s = p->next;
			p->next = q->next;
			q->next = p;
			p = s;
			q = pAET;
		}


		p = pAET->next;
		while (p != NULL && p->next != NULL)
		{
			for (float j = p->x; j <= p->next->x; j++)
			{
				image.set(static_cast<int>(j), i, this_color);
			}  // pDC.MoveTo( static_cast<int>(p->x), i ); �û�ֱ�����滻������������ص���ɫ���ٶȸ���
						   //  pDC.LineTo( static_cast<int>(p->next->x), i );

			p = p->next->next;//���Ƕ˵����
		}
	}

	ET* phead = NULL;
	ET* pnext = NULL;
	//�ͷű߱�
	/*
	for( i = MinY;i <= MaxY;i++ ) //��������ͷ��ڴ��е����
	{
		phead = pNET[i];
		while( phead != NULL )
		{
			pnext = phead->next;
			delete phead;
			phead = pnext;
		}
		pNET[i] = NULL;
	}
	*/
	//�ͷŻ�Ծ�߱�
	phead = pAET;
	while (phead != NULL)
	{
		pnext = phead->next;
		delete phead;
		phead = pnext;
	}
}