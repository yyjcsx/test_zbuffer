#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include<iostream>
#include<memory>

const int height = 800;
const int width = 800;
const int POINTNUM = 3;
const TGAColor white = TGAColor(255, 255, 255, 255);


//定义分类的边表
struct ET {
	float x;
	float xmin;
	float dx;
	float dy;
	float ymax;
	float z;
	float ymin;
	int id_of_f;
	ET* next;
	ET();
};
ET::ET() { next = nullptr; }


//定义分类多边形表和活化的多边形表
typedef struct XF {
	float a;
	float b;
	float c;
	float d;
	float ymax;
	float ymin;
	int id;
	int dy;
	std::vector<ET*> T_in_F;
	TGAColor color;
	XF* next;

}EF, AEF;

//定义活化边表
struct AET {
	float xl, dxl, xr, dxr, zl, dzx, dzy, yl_max, yr_max, yl_min, yr_min;
	int dyl, dyr;
	int id;
	TGAColor color;
	AET* next;
	ET* next_T;
};


Vec3f light_dir(0, 0, -1);
void zbuffer(Model* model, TGAImage& image) {
	//static ET* m_ET[2200][height];
	int check_number = 0;
	std::vector<EF*> id_and_color;
	//进行多边形表的定义
	const int number_of_faces = model->nfaces();
	std::vector<EF*> pEF(height + 1, nullptr);
	EF* EFbridge = nullptr;
	//进行分类多边形边表的定义
	std::vector<std::vector<ET*>> pET(number_of_faces, std::vector<ET*>(height + 1, nullptr));

	//进行活化多边形链表的初始化，头只有一个next指向第一个元素，初始化为nullptr
	std::shared_ptr <AEF> pAEF = std::make_shared<AEF>();
	//AEF* pAEF = new AEF;
	pAEF->next = nullptr;
	//进行活化边表的初始化，同上
	std::shared_ptr <AET> pAET = std::make_shared<AET>();
	//AET* pAET = new AET;
	pAET->next = nullptr;
	int total_y_max = 0;
	int total_y_min = 9999;
	////开始定义z值缓存

	static float m_ZB[width];//深度缓存
	//建立顶点和多边形的关系
	std::vector<std::vector<Vec3f>> T_of_the_f;
	int k = 0;
	//对每个面进行遍历，建立边表和多边形表
	for (int i = 0; i < model->nfaces(); i++) {
		k++;
		//对model进行遍历，得到每一个边的序号，id就用i吧
		std::vector<int> face = model->face(i);//i是表示多边形面数的序号
		std::vector<Vec3f> model_face;
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
		//定义三个顶点
		Vec3f v0;
		Vec3f v1;
		Vec3f v2;
		//确定Ymax，Ymin，这是单个多边形的Ymax，Ymin
		int Y_max = 0;
		int Y_min = 99999;
		for (int j = 0; j < 3; j++) {
			v0 = model->vert(face[j]);
			v1 = model->vert(face[(j + 1) % 3]);//因为本文的obj文件全部都是三角形，所以直接用3代替，可以进行扩展
			v2 = model->vert(face[(j + 2) % 3]);
			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (v0.y + 1.) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (v1.y + 1.) * height / 2.;
			int x2 = (v2.x + 1.) * width / 2.;
			int y2 = (v2.y + 1.) * height / 2.;
			v0.x = x0; v0.y = y0;
			v1.x = x1; v1.y = y1;
			v2.x = x2; v2.y = y2;
			model_face.push_back(v0);
			if (model_face[j].y > Y_max) {
				Y_max = model_face[j].y;
				if (Y_max > total_y_max)
					total_y_max = Y_max;
			}
			if (model_face[j].y < Y_min) {
				Y_min = model_face[j].y;
				if (Y_min < total_y_min)
					total_y_min = Y_min;
			}
		}
		//开始填充多边形表EF	
		float A = (v2.y - v0.y) * (v2.z - v0.z) - (v1.z - v0.z) * (v2.y - v0.y);
		float B = (v2.x - v0.x) * (v1.z - v0.z) - (v1.x - v0.x) * (v2.z - v0.z);
		float C = (v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y);
		float D = -(A * v0.x + B * v0.y + C * v0.z);
		EF* f = new EF;
		f->a = A;
		f->b = B;
		f->c = C;
		f->d = D;
		f->id = i;
		f->dy = Y_max - Y_min;
		f->ymax = Y_max;
		f->ymin = Y_min;
		f->color = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);//给他一个随机的颜色
		f->next = nullptr;
		id_and_color.push_back(f);
		T_of_the_f.push_back(model_face);
		if (pEF[Y_max] == nullptr)
		{
			pEF[Y_max] = new EF;
			pEF[Y_max]->next = f;
		}
		else {

			EF* p = pEF[Y_max];
			while (p->next != nullptr) p = p->next;
			p->next = f;
		}
	}

	int test_number = 0;
	//正式开始算法，从上到下扫描
	int num = 0;
	for (int y_scan = total_y_max; y_scan >= total_y_min; --y_scan) {
		//	帧缓冲器设置为底色，z缓冲器各单元为最小值
		for (int i_w = 0; i_w < width; i_w++)
		{
			m_ZB[i_w] = -999999999.;//定义每一行的z值
		}
		//如果y表中有扫描线i对应的扫描线，则将其加入到APT中
		//明确一下，PET边表；PEF多边形表；pAEF活化多边形表；都是第一个元素不装数据只有next指针有意义
		if (pEF[y_scan] != nullptr)
		{
			std::cout << "y_scan is " << y_scan << std::endl;
			EF* head_of_EF = pEF[y_scan]->next;
			EF* head_of_APF = pAEF->next;
			int unit_number_of_AEF = 0;//计数，看一根扫描线能对应多少个多边形

			//将head_of_APF延伸到APF的最尾端
			if (head_of_APF != nullptr)
				while (head_of_APF->next != nullptr)
					head_of_APF = head_of_APF->next;

			while (head_of_EF != nullptr) {
				EF* pEF_temp = new EF;
				pEF_temp = head_of_EF;
				if (pAEF->next == nullptr) {//如果活化多边形表为空，就把多边形表里头节点之后第一个值赋给活化多边形表
					pAEF->next = pEF_temp;
					head_of_APF = pAEF->next;
				}
				else {//如果不为空，就在headofAPF后面加一个，让他往后挪一位
					head_of_APF->next = pEF_temp;
					head_of_APF = head_of_APF->next;
				}
				for (int j1 = 0; j1 < 3; j1++) {
					ET* bridge = nullptr;
					ET* bridge2 = nullptr;
					if (T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y > T_of_the_f[head_of_APF->id][j1].y)
					{
						ET* p = new ET;
						p->xmin = T_of_the_f[head_of_APF->id][j1].x;//这里是较低顶点的x坐标
						p->x = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].x;//修改：这里是较高顶点的x坐标
						//p->z = model_face[j1].z;//较低顶点的z坐标
						p->z = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].z;//修改：这里是较高顶点的z坐标
						p->ymax = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y;
						p->ymin = T_of_the_f[head_of_APF->id][j1].y;
						p->dx = ((T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].x - T_of_the_f[head_of_APF->id][j1].x) / (T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y - T_of_the_f[head_of_APF->id][j1].y));
						//将对应边加入边表
						if (pET[head_of_APF->id][T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y] == nullptr)
							pET[head_of_APF->id][T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y] = p;
						else
							pET[head_of_APF->id][T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y]->next = p;

					}
					else if (T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y < T_of_the_f[head_of_APF->id][j1].y)
					{
						ET* p = new ET;
						p->xmin = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].x;
						p->x = T_of_the_f[head_of_APF->id][j1].x;//修改：这里是较高顶点的x坐标
						//p->z = model_face[j1].z;//较低顶点的z坐标
						p->z = T_of_the_f[head_of_APF->id][j1].z;//修改：这里是较高顶点的z坐标
						p->ymax = T_of_the_f[head_of_APF->id][j1].y;
						p->ymin = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y;
						p->dx = ((T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].x - T_of_the_f[head_of_APF->id][j1].x) / (T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y - T_of_the_f[head_of_APF->id][j1].y));

						//将对应边加入边表
						if (pET[head_of_APF->id][T_of_the_f[head_of_APF->id][j1].y] == nullptr)
							pET[head_of_APF->id][T_of_the_f[head_of_APF->id][j1].y] = p;
						else
							pET[head_of_APF->id][T_of_the_f[head_of_APF->id][j1].y]->next = p;
					}

				}
				//挪一个之后，多边形表向后挪一位准备将下一个ymax的多边形加到活化边表中
				head_of_EF = head_of_EF->next;
			}

			//尝试让headofAPF重新指到活化多边形表表头，然后依次向后指
			head_of_APF = pAEF->next;
			while (head_of_APF != nullptr) {
				if (pET[head_of_APF->id][y_scan] != nullptr)
				{
					//std::cout << head_of_APF->id << "is out to work" << std::endl;
					if (pET[head_of_APF->id][y_scan]->next != nullptr)

					{
						//std::cout << "work" << std::endl;
						ET edge1;
						ET edge2;
						AET* newAET = new AET();
						edge1 = *pET[head_of_APF->id][y_scan];
						edge2 = *pET[head_of_APF->id][y_scan]->next;
						int flag = 0;
						//判断哪个边在左边哪个边在右边
						if (edge1.xmin > edge2.xmin) {
							ET temp;
							temp = edge1;
							edge1 = edge2;
							edge2 = temp;
						}
						else
							if (edge1.dx < edge2.dx) {//这里判断方法存疑？？哪边斜率倒数比较大哪边就是左侧
								ET temp;
								temp = edge1;
								edge1 = edge2;
								edge2 = temp;
							}
						newAET->dxl = edge1.dx;
						newAET->dxr = edge2.dx;

						newAET->xl = edge1.x;
						newAET->xr = edge2.x;
						newAET->zl = edge1.z;
						newAET->yl_max = edge1.ymax;
						newAET->yr_max = edge2.ymax;
						newAET->yl_min = edge1.ymin;
						newAET->yr_min = edge2.ymin;
						newAET->id = head_of_APF->id;
						newAET->next = nullptr;
						newAET->dzx = -(head_of_APF->a) / (head_of_APF->c);
						newAET->dzy = head_of_APF->b / head_of_APF->c;

						if (pAET->next == nullptr)//如果活化边对表为空则直接在head后面添加
							pAET->next = newAET;
						else {//如果不为空则先找到边对表的最末尾，在最末尾添加
							AET* head_of_AET = pAET->next;
							while (head_of_AET->next != nullptr)
								head_of_AET = head_of_AET->next;
							head_of_AET->next = newAET;
							head_of_AET = head_of_AET->next;

						}
					}
				}
				head_of_APF = head_of_APF->next;
			}
		}

		AET* head_of_AET = pAET->next;

		while (head_of_AET != nullptr) {

			if (head_of_AET->xl >= 0 && head_of_AET->xl <= 800 && head_of_AET->xr >= 0 && head_of_AET->xr <= 800)
				for (int j = head_of_AET->xl; j <= head_of_AET->xr && j <= 800; j++)
				{
					if (head_of_AET->zl + head_of_AET->dzx * (j - head_of_AET->xl) > m_ZB[j])
					{
						m_ZB[j] = head_of_AET->zl + head_of_AET->dzx * (j - head_of_AET->xl);
						image.set(static_cast<int>(j), y_scan, id_and_color[head_of_AET->id]->color);
					}
				}
			head_of_AET = head_of_AET->next;
		}
		//删除已经遍历完的多边形
		EF* head_of_APF = pAEF->next;
		while (head_of_APF != nullptr && head_of_APF->ymin >= y_scan) {//这里扫描顺序和ymax/min的选择要搞清楚
			pAEF->next = head_of_APF->next;
			free(head_of_APF);
			head_of_APF = pAEF->next;
		}

		//将head_of_APF的位置不断向next靠，直到pAEF的最前端一个
		while (head_of_APF != nullptr && head_of_APF->next != nullptr) {
			if (head_of_APF->next->ymin >= y_scan) {
				EF* next = head_of_APF->next;
				head_of_APF->next = next->next;
				free(next);
			}
			else {
				head_of_APF = head_of_APF->next;
			}
		}
		//更新活化边对表中的边对
		head_of_AET = pAET->next;
		//先删除首元素就不在扫描范围内的情况
		while (head_of_AET != nullptr && head_of_AET->yl_min >= y_scan && head_of_AET->yr_min >= y_scan)
		{

			pAET->next = head_of_AET->next;
			free(head_of_AET);
			head_of_AET = pAET->next;
		}
		//删除后面元素全部不在扫描范围内的情况
		while (head_of_AET != nullptr && head_of_AET->next != nullptr) {
			if (head_of_AET->next->yl_min >= y_scan && head_of_AET->next->yr_min >= y_scan) {
				AET* next = head_of_AET->next;
				head_of_AET->next = next->next;
				free(next);
			}
			else
				head_of_AET = head_of_AET->next;
		}
		head_of_AET = pAET->next;
		while (head_of_AET != nullptr)
		{
			if (head_of_AET->yl_min >= y_scan)
			{
				if (pET[head_of_AET->id][y_scan] != nullptr)//由pAET->next修改为headof_AET
				{
					head_of_AET->yl_min = pET[head_of_AET->id][y_scan]->ymin;
					head_of_AET->dxl = pET[head_of_AET->id][y_scan]->dx;
					head_of_AET->zl = pET[head_of_AET->id][y_scan]->z;
					head_of_AET->xl = pET[head_of_AET->id][y_scan]->x;//x坐标是上端点的
				}
			}
			else if (head_of_AET->yr_min >= y_scan)
			{
				if (pET[head_of_AET->id][y_scan] != nullptr)
				{
					head_of_AET->yr_min = pET[head_of_AET->id][y_scan]->ymin;
					head_of_AET->dxr = pET[head_of_AET->id][y_scan]->dx;
					head_of_AET->xr = pET[head_of_AET->id][y_scan]->x;//x坐标是上端点的
				}
			}
			head_of_AET->xl -= head_of_AET->dxl;
			head_of_AET->xr -= head_of_AET->dxr;//修改，由从上往下扫描的特性把+=变成了-=，通过斜率和几何关系可以证明
			head_of_AET->zl += -head_of_AET->dxl * head_of_AET->dzx + head_of_AET->dzy;
			head_of_AET = head_of_AET->next;
		}
	}
}
