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


//�������ı߱�
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


//����������α�ͻ�Ķ���α�
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

//�����߱�
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
	//���ж���α�Ķ���
	const int number_of_faces = model->nfaces();
	std::vector<EF*> pEF(height + 1, nullptr);
	EF* EFbridge = nullptr;
	//���з������α߱�Ķ���
	std::vector<std::vector<ET*>> pET(number_of_faces, std::vector<ET*>(height + 1, nullptr));

	//���л���������ĳ�ʼ����ͷֻ��һ��nextָ���һ��Ԫ�أ���ʼ��Ϊnullptr
	std::shared_ptr <AEF> pAEF = std::make_shared<AEF>();
	//AEF* pAEF = new AEF;
	pAEF->next = nullptr;
	//���л�߱�ĳ�ʼ����ͬ��
	std::shared_ptr <AET> pAET = std::make_shared<AET>();
	//AET* pAET = new AET;
	pAET->next = nullptr;
	int total_y_max = 0;
	int total_y_min = 9999;
	////��ʼ����zֵ����

	static float m_ZB[width];//��Ȼ���
	//��������Ͷ���εĹ�ϵ
	std::vector<std::vector<Vec3f>> T_of_the_f;
	int k = 0;
	//��ÿ������б����������߱�Ͷ���α�
	for (int i = 0; i < model->nfaces(); i++) {
		k++;
		//��model���б������õ�ÿһ���ߵ���ţ�id����i��
		std::vector<int> face = model->face(i);//i�Ǳ�ʾ��������������
		std::vector<Vec3f> model_face;
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
		//������������
		Vec3f v0;
		Vec3f v1;
		Vec3f v2;
		//ȷ��Ymax��Ymin�����ǵ�������ε�Ymax��Ymin
		int Y_max = 0;
		int Y_min = 99999;
		for (int j = 0; j < 3; j++) {
			v0 = model->vert(face[j]);
			v1 = model->vert(face[(j + 1) % 3]);//��Ϊ���ĵ�obj�ļ�ȫ�����������Σ�����ֱ����3���棬���Խ�����չ
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
		//��ʼ������α�EF	
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
		f->color = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);//����һ���������ɫ
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
	//��ʽ��ʼ�㷨�����ϵ���ɨ��
	int num = 0;
	for (int y_scan = total_y_max; y_scan >= total_y_min; --y_scan) {
		//	֡����������Ϊ��ɫ��z����������ԪΪ��Сֵ
		for (int i_w = 0; i_w < width; i_w++)
		{
			m_ZB[i_w] = -999999999.;//����ÿһ�е�zֵ
		}
		//���y������ɨ����i��Ӧ��ɨ���ߣ�������뵽APT��
		//��ȷһ�£�PET�߱�PEF����α�pAEF�����α����ǵ�һ��Ԫ�ز�װ����ֻ��nextָ��������
		if (pEF[y_scan] != nullptr)
		{
			std::cout << "y_scan is " << y_scan << std::endl;
			EF* head_of_EF = pEF[y_scan]->next;
			EF* head_of_APF = pAEF->next;
			int unit_number_of_AEF = 0;//��������һ��ɨ�����ܶ�Ӧ���ٸ������

			//��head_of_APF���쵽APF����β��
			if (head_of_APF != nullptr)
				while (head_of_APF->next != nullptr)
					head_of_APF = head_of_APF->next;

			while (head_of_EF != nullptr) {
				EF* pEF_temp = new EF;
				pEF_temp = head_of_EF;
				if (pAEF->next == nullptr) {//��������α�Ϊ�գ��ͰѶ���α���ͷ�ڵ�֮���һ��ֵ���������α�
					pAEF->next = pEF_temp;
					head_of_APF = pAEF->next;
				}
				else {//�����Ϊ�գ�����headofAPF�����һ������������Ųһλ
					head_of_APF->next = pEF_temp;
					head_of_APF = head_of_APF->next;
				}
				for (int j1 = 0; j1 < 3; j1++) {
					ET* bridge = nullptr;
					ET* bridge2 = nullptr;
					if (T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y > T_of_the_f[head_of_APF->id][j1].y)
					{
						ET* p = new ET;
						p->xmin = T_of_the_f[head_of_APF->id][j1].x;//�����ǽϵͶ����x����
						p->x = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].x;//�޸ģ������ǽϸ߶����x����
						//p->z = model_face[j1].z;//�ϵͶ����z����
						p->z = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].z;//�޸ģ������ǽϸ߶����z����
						p->ymax = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y;
						p->ymin = T_of_the_f[head_of_APF->id][j1].y;
						p->dx = ((T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].x - T_of_the_f[head_of_APF->id][j1].x) / (T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y - T_of_the_f[head_of_APF->id][j1].y));
						//����Ӧ�߼���߱�
						if (pET[head_of_APF->id][T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y] == nullptr)
							pET[head_of_APF->id][T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y] = p;
						else
							pET[head_of_APF->id][T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y]->next = p;

					}
					else if (T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y < T_of_the_f[head_of_APF->id][j1].y)
					{
						ET* p = new ET;
						p->xmin = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].x;
						p->x = T_of_the_f[head_of_APF->id][j1].x;//�޸ģ������ǽϸ߶����x����
						//p->z = model_face[j1].z;//�ϵͶ����z����
						p->z = T_of_the_f[head_of_APF->id][j1].z;//�޸ģ������ǽϸ߶����z����
						p->ymax = T_of_the_f[head_of_APF->id][j1].y;
						p->ymin = T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y;
						p->dx = ((T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].x - T_of_the_f[head_of_APF->id][j1].x) / (T_of_the_f[head_of_APF->id][(j1 + 1 + POINTNUM) % POINTNUM].y - T_of_the_f[head_of_APF->id][j1].y));

						//����Ӧ�߼���߱�
						if (pET[head_of_APF->id][T_of_the_f[head_of_APF->id][j1].y] == nullptr)
							pET[head_of_APF->id][T_of_the_f[head_of_APF->id][j1].y] = p;
						else
							pET[head_of_APF->id][T_of_the_f[head_of_APF->id][j1].y]->next = p;
					}

				}
				//Ųһ��֮�󣬶���α����Ųһλ׼������һ��ymax�Ķ���μӵ���߱���
				head_of_EF = head_of_EF->next;
			}

			//������headofAPF����ָ�������α��ͷ��Ȼ���������ָ
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
						//�ж��ĸ���������ĸ������ұ�
						if (edge1.xmin > edge2.xmin) {
							ET temp;
							temp = edge1;
							edge1 = edge2;
							edge2 = temp;
						}
						else
							if (edge1.dx < edge2.dx) {//�����жϷ������ɣ����ı�б�ʵ����Ƚϴ��ı߾������
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

						if (pAET->next == nullptr)//�����߶Ա�Ϊ����ֱ����head�������
							pAET->next = newAET;
						else {//�����Ϊ�������ҵ��߶Ա����ĩβ������ĩβ���
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
		//ɾ���Ѿ�������Ķ����
		EF* head_of_APF = pAEF->next;
		while (head_of_APF != nullptr && head_of_APF->ymin >= y_scan) {//����ɨ��˳���ymax/min��ѡ��Ҫ�����
			pAEF->next = head_of_APF->next;
			free(head_of_APF);
			head_of_APF = pAEF->next;
		}

		//��head_of_APF��λ�ò�����next����ֱ��pAEF����ǰ��һ��
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
		//���»�߶Ա��еı߶�
		head_of_AET = pAET->next;
		//��ɾ����Ԫ�ؾͲ���ɨ�跶Χ�ڵ����
		while (head_of_AET != nullptr && head_of_AET->yl_min >= y_scan && head_of_AET->yr_min >= y_scan)
		{

			pAET->next = head_of_AET->next;
			free(head_of_AET);
			head_of_AET = pAET->next;
		}
		//ɾ������Ԫ��ȫ������ɨ�跶Χ�ڵ����
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
				if (pET[head_of_AET->id][y_scan] != nullptr)//��pAET->next�޸�Ϊheadof_AET
				{
					head_of_AET->yl_min = pET[head_of_AET->id][y_scan]->ymin;
					head_of_AET->dxl = pET[head_of_AET->id][y_scan]->dx;
					head_of_AET->zl = pET[head_of_AET->id][y_scan]->z;
					head_of_AET->xl = pET[head_of_AET->id][y_scan]->x;//x�������϶˵��
				}
			}
			else if (head_of_AET->yr_min >= y_scan)
			{
				if (pET[head_of_AET->id][y_scan] != nullptr)
				{
					head_of_AET->yr_min = pET[head_of_AET->id][y_scan]->ymin;
					head_of_AET->dxr = pET[head_of_AET->id][y_scan]->dx;
					head_of_AET->xr = pET[head_of_AET->id][y_scan]->x;//x�������϶˵��
				}
			}
			head_of_AET->xl -= head_of_AET->dxl;
			head_of_AET->xr -= head_of_AET->dxr;//�޸ģ��ɴ�������ɨ������԰�+=�����-=��ͨ��б�ʺͼ��ι�ϵ����֤��
			head_of_AET->zl += -head_of_AET->dxl * head_of_AET->dzx + head_of_AET->dzy;
			head_of_AET = head_of_AET->next;
		}
	}
}
