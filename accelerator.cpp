/*********************************************
 *
 *	 Accelerator Lab
 *
 ********************************************/

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <stdio.h>
#include <string.h>

#define MAX_COLLIDERS 	28
#define MAX_POINTS 		82
#define MAX_DEVICES 	82
#define MAX_COLLISIONS  99999

using namespace std;

FILE *f;
int n_points, n_colliders, n_devices, n_collisions;
int place[MAX_POINTS][3];					// x; y; 1 ocupado/ 0 n ocupado;
int device[MAX_DEVICES];				    // guarda o ponto actual do device
int device_pos_best[MAX_DEVICES];		    // guarda melhor posicao
int collider[MAX_COLLIDERS][2];             // p1 | p2
bool map[9][9]={{false}};
bool debug=false;
int last_check=0;
int rot1,rot2,rot3,rot4;

char collisions[MAX_POINTS*MAX_POINTS][MAX_POINTS*MAX_POINTS];  // '0' n calculado
                                                                // '1' intersecta
                                                                // '2' n intersecta
int count_int,ultimo_pos,i_conta,j_conta,ultimo=0;      // vars do conta_inter

// print das posicoes para um ficheiro (INCOMPLETA AQUI)
void print_colliders(){
	if(f!=NULL){
		for(int i = 0; i < n_colliders; i++){
			fprintf(f, "%d | %d\n",device[collider[i][0]],device[collider[i][1]]);
		}
		fputs("_________________\n",f);
	}
}

// faz sort aos colliders
void bubble_sort(){
	int i, j,tmp0,tmp1;
	for (i = 0; i < n_colliders-1; i++){
    	for (j = 0; j < n_colliders-i-1; j++){
        	if (collider[j][0] > collider[j+1][0]){
				tmp0=collider[j][0];
				tmp1=collider[j][1];
				collider[j][0]=collider[j+1][0];
				collider[j][1]=collider[j+1][1];
				collider[j+1][0]=tmp0;
				collider[j+1][1]=tmp1;
            }
		}
	}
}

// retorna: 0 se sao colineares, 1 se for clockwise,
// 			-1 se for counter clockwise
int rotacao(int x1, int y1, int x2, int y2, int x3, int y3){
	int rot = (y2-y1)*(x3-x2)-(x2-x1)*(y3-y2);
  	if(rot>0)
  		return 1;
  	else if(rot<0)
  		return -1;
	return 0;
}
float m,b,y;
// retorna true se o ponto do q esta entre os outros dois
bool pertence(int x1, int y1, int x2, int y2, int x3, int y3){
	if(x3==x1 && x1==x2 && y2<max(y1,y3) && y2>min(y1,y3))
		return true;
	if(y3==y1 && y1==y2 && x2<max(x1,x3) && x2>min(x1,x3)){
		return true;
	}
	if(x2>min(x1,x3) && x2<max(x1,x3) && y2>min(y1,y3) && y2<max(y1,y3)){
		m=(y3-y1)/(x3-x1);
		b=y1-(m*x1);
		y=m*x2+b;
		if((int)y==y2){
			return true;
		}
	}
	return false;
}

// retorna true se x1y1-xx1yy1 intersecta x2y2-xx2yy2, senao false
bool intersecta(int x1, int y1, int xx1, int yy1, int x2, int y2, int xx2,int yy2){
	// casos em que partilham uma extremidade
	if((x1==x2 && y1==y2) || (xx1==xx2 && yy1==yy2) || (x1==xx2 && y1==yy2) || (x2==xx1 && y2==yy1))
		return false;
    // calcula os sentidos dos pontos
	rot1 = rotacao(x1,y1, xx1,yy1, x2,y2);
	rot2 = rotacao(x1,y1, xx1,yy1, xx2,yy2);
	rot3 = rotacao(x2,y2, xx2,yy2, x1,y1);
	rot4 = rotacao(x2,y2, xx2,yy2, xx1,yy1);
	// Se o sentido de rotacao entre xy1->xxyy1->xy2 != xy1->xxyy1->xxyy2
	// 			   e a rotacao entre xy2->xxyy2->xy1 != xy2->xxyy2->xxyy1
	if (rot1 != rot2 && rot3 != rot4)
		return true;

	// Caso sejam colineares
	if (rot1 == 0 && pertence(x1,y1, x2,y2, xx1,yy1))
		return true;
	if (rot2 == 0 && pertence(x1,y1, xx2,yy2, xx1,yy1))
		return true;
	if (rot3 == 0 && pertence(x2,y2, x1,y1, xx2,yy2))
		return true;
	if (rot4 == 0 && pertence(x2,y2, xx1,yy1, xx2,yy2))
		return true;

	return false;
}

// verifica inter. entre dois segmentos
int s1,s2;
bool verifica_inter(int p1,int p2, int q1, int q2){
    s1 = p1*n_points+p2;
    s2 = q1*n_points+q2;
    if(collisions[s1][s2]=='1')
        return true;
    else if(collisions[s1][s2]=='2')
        return false;
    else{
        if(intersecta(place[p1][0],place[p1][1], place[p2][0], place[p2][1],
                        place[q1][0], place[q1][1], place[q2][0], place[q2][1])){
            collisions[s1][s2]='1';
            collisions[s2][s1]='1';
            return true;
        }else{
            collisions[s1][s2]='2';
            collisions[s2][s1]='2';
            return false;
        }
    }
    return false;
}

// conta as interseccoes existentes ate ao dev
void conta_inter(int dev,int count_coll){
	count_int=0;
	if(ultimo==0){
		for(i_conta=0; i_conta<n_colliders;i_conta++){
			if(collider[i_conta][0]==dev){
				ultimo=i_conta;
				break;
			}
		}
	}
	if(ultimo>0){
	    ultimo_pos=ultimo;
		while(ultimo<n_colliders && collider[ultimo][0]==dev){
			for(j_conta=0;j_conta<ultimo_pos;j_conta++){
				if(verifica_inter(device[collider[ultimo][0]],device[collider[ultimo][1]],device[collider[j_conta][0]],device[collider[j_conta][1]])){
					count_int++;
					if((count_int+count_coll)>=n_collisions){
						count_int=MAX_COLLISIONS;
						return;
					}
				}
			}
			ultimo++;
		}
	}
}

// Testa todas as combinacoes possiveis de posicionamento dos devices
void combina(int dev,int count_coll){
    conta_inter(dev,count_coll);
    int ci=count_int+count_coll;	
	if(ci<n_collisions){			// se o nosso contador local de intersecções é inferior ao contador global
		if(dev==n_devices){			// e já colocámos todos os devices
			n_collisions=ci;		// contador global actualiza
			if(debug){
				print_colliders();
				memcpy(&device_pos_best,&device,sizeof(device));
			}
			ultimo=0;
			return;
		}
	}else{
	    ultimo=0;
		return;
    }

	for(int ponto=0; ponto<n_points; ponto++){
		if(place[ponto][2]==0){
			place[ponto][2]=1;
			device[dev+1]=ponto;
			combina(dev+1,ci);
			place[ponto][2]=0;
		}
	}
}

int main(int argc, char *argv[]){
    int i;
	if(argc==2)
		if(strncmp(argv[1],"-d",2)==0){
			f = fopen("data.out","w");
			debug = true;
		}

    memset(*collisions,'0',sizeof(collisions));

	cin >> n_points;
	for(i = 0; i < n_points; i++){
		cin >> place[i][0] >> place[i][1];
	}

	cin >> n_devices >> n_colliders;
	int a,b;
	for(i = 0; i < n_colliders; i++){
	    cin >> a >> b;
		if(a>b){
		    collider[i][0]=a;
		    collider[i][1]=b;
		}else{
		    collider[i][0]=b;
		    collider[i][1]=a;
		}
	}
	bubble_sort();						// ordenação dos coliders verticalmente por ordem de devices

	n_collisions = MAX_COLLISIONS; 	    // inicializar as colisoes no num max possivel
	if(n_devices>3){				    // apenas se os devices forem mais do q 3 havera alguma colisao
		for(i = 0; i < n_points; i++){
			if(n_collisions==0)
				break;
			place[i][2]=1;              // marcamos o ponto como utilizado
			device[1]=i;                // atribuimos o ponto ao device
			combina(1,0);				// CHAMADA COMBINA
			place[i][2]=0;              // desmarcamos o ponto
		}
	}else							    // caso contrario n_collisions vai ser sempre zero
		n_collisions=0;

	if(debug){
		cout<<"Best:"<<endl;
		for(i = 1 ; i<= n_devices;i++){
			cout << i<<" ("<< place[device_pos_best[i]][0]<<","<< place[device_pos_best[i]][1]<<")"<<endl;
		}
	}
	cout << n_collisions << endl;

	if(f!=NULL)
		fclose(f);

	return 0;
}


