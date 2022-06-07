#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include<iostream>
#include<memory>

const int POINTNUM = 3;

//定义AET，ET的基本结构
typedef struct XET {
	float x;
	float dx, ymax;
	XET* next;

}AET, ET;





void draw_color(std::vector<Vec3f>& model_face, TGAImage& image) {
	auto this_color = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);



	//扫描一下计算ymin和ymax
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
	//进行活化边表的初始化
	AET* pAET = new AET;
	pAET->next = NULL;
	//进行边表的初始化
	ET* pET[2048];//这个就是PPT上面竖着的那一排
	for (i = Y_min; i <= Y_max; ++i) {
		pET[i] = new ET;
		pET[i]->next = NULL;
		//pET[i] = NULL;
	}

	//扫描并建立边表
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


		//先更新AET中的数据，空的话就不用管直接NULL了，主要针对运行到一半时候的非空情况
		ET* p = pAET->next;
		while (p != NULL) {
			p->x = p->x + p->dx;
			p = p->next;
		}


		//AET排序
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
			}  // pDC.MoveTo( static_cast<int>(p->x), i ); 用画直线来替换上面的设置像素点颜色，速度更快
						   //  pDC.LineTo( static_cast<int>(p->next->x), i );

			p = p->next->next;//考虑端点情况
		}
	}

	ET* phead = NULL;
	ET* pnext = NULL;
	//释放边表
	/*
	for( i = MinY;i <= MaxY;i++ ) //下面代码释放内存有点错误
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
	//释放活跃边表
	phead = pAET;
	while (phead != NULL)
	{
		pnext = phead->next;
		delete phead;
		phead = pnext;
	}
}