#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <vector>
#include <signal.h>
#include <wait.h>

using namespace std;

#define NOMBRE_PROCESOS "hSuma", "hResta", "hProducto", "hDividir"
#define CANT_ELEMENTOS 10
#define NOMBRE_MEMORIA "memoriaDatos"

int* obtenerPuntMemoria ( char * nomMemoria, int cantElem ){
    int *mem;
    int idMemoria = shm_open( nomMemoria, O_CREAT | O_RDWR, 0600 );
    ftruncate( idMemoria, sizeof(int) * cantElem );

    mem = (int*) mmap( NULL, sizeof(int) * CANT_ELEMENTOS, PROT_READ | PROT_WRITE, MAP_SHARED, idMemoria, 0);
    close(idMemoria);
    return mem;
}
 bool* obtenerBand(){
    int idBand =  shm_open( "bandFin", O_CREAT | O_RDWR, 0600 );
    ftruncate( idBand, sizeof(bool));
    bool *band = (bool*) mmap( NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED, idBand, 0);
    close(idBand);
    return band;
 }

 void crearSemaforos ( sem_t *semaforos[], vector<string>nombres ){
    semaforos[0] = sem_open(  nombres[0].c_str(), O_CREAT, 0600, 0 );
    semaforos[1] = sem_open(  nombres[1].c_str(), O_CREAT, 0600, 1 );
    semaforos[2] = sem_open(  nombres[2].c_str(), O_CREAT, 0600, 1 );
    semaforos[3] = sem_open(  nombres[3].c_str(), O_CREAT, 0600, 1 );
 }

void verMemoria( int* array ){
    for (size_t i = 0; i < CANT_ELEMENTOS; i++)
    {
        cout<<"Pos: "<<i<<" valor: "<<array[i]<<endl;
    }
    
}

int main(  )
{
    vector<string> procHijos = {NOMBRE_PROCESOS};
    sem_t* semaforos[ procHijos.size() ];
    pid_t procesos[ procHijos.size() ]; 
    int *memoria;
    bool *fin;
    //Crear los semaforos
    sem_unlink( procHijos[0].c_str() );
    sem_unlink( procHijos[1].c_str() );
    sem_unlink( procHijos[2].c_str() );
    sem_unlink( procHijos[3].c_str() );
    crearSemaforos( semaforos, procHijos );//lo creamos de tla forma que ejecute primero el del hijo

    memoria = obtenerPuntMemoria( (char*)NOMBRE_MEMORIA, CANT_ELEMENTOS);
    fin = obtenerBand();

    procesos[0] = fork();//creo el proceso de suma en pos inpares
    if( procesos[0] == 0 ) {
        int sumIndex = 1;
        int proximo = 2;
        while( ! *fin )
        {
            sem_wait( semaforos[1] );
            sem_wait( semaforos[2] );
            sem_wait( semaforos[3] );
            memoria[sumIndex] = ( memoria[sumIndex] + memoria[proximo] > __INT_MAX__ ? rand() : memoria[sumIndex] + memoria[proximo] );
            sem_post( semaforos[0] );
            sem_post( semaforos[1] );
            sem_post( semaforos[2] );
            sem_post( semaforos[3] );
            proximo = (proximo + 2) % CANT_ELEMENTOS ;
            sumIndex = (sumIndex + 2) % CANT_ELEMENTOS;
        }
    }
    else 
        if( procesos[0] > 0 ){
            procesos[1] = fork();//creo el proceso resta pos pares
            if ( procesos[1] == 0 )
            {
                int restaIndex = 0;
                int anterior =  CANT_ELEMENTOS - 1;
                    while( ! *fin ){
                        
                        sem_wait( semaforos[0] );
                        sem_wait( semaforos[2] );
                        sem_wait( semaforos[3] );
                        memoria[restaIndex] = ( memoria[restaIndex] - memoria[anterior] < _SC_INT_MIN ? rand() : memoria[restaIndex] - memoria[anterior] );
                        sem_post( semaforos[0] );
                        sem_post( semaforos[1] );
                        sem_post( semaforos[2] );
                        sem_post( semaforos[3] );
                        anterior = (anterior + 2) % CANT_ELEMENTOS;
                        restaIndex = ( restaIndex + 2 ) % CANT_ELEMENTOS;
                    }
            }
            else
            if( procesos[0] > 0 && procesos[1] > 0){
                procesos[2] = fork(); //creo el proceso producto en pos pares
                if( procesos[2] == 0 ){
                    int proIndex = 0;
                    int proximo = CANT_ELEMENTOS - 1;
                        while( ! *fin )
                        {
                                sem_wait( semaforos[0] );
                                sem_wait( semaforos[1] );
                                sem_wait( semaforos[3] );
                                memoria[proIndex] = ( memoria[proIndex] * ( memoria[proximo] * 0.1 ) > __INT_MAX__ ? rand() : memoria[proIndex] * ( memoria[proximo] * 0.1 ) );
                                sem_post( semaforos[0] );
                                sem_post( semaforos[1] );
                                sem_post( semaforos[2] );
                                sem_post( semaforos[3] );
                            proximo = (proximo +2 ) % CANT_ELEMENTOS;
                            proIndex = (proIndex + 2 ) %CANT_ELEMENTOS;
                        }
                    }
                    else
                    if(procesos[0] > 0 && procesos[1] > 0 && procesos[2] > 0  ){
                                procesos[3] = fork(); //creo el proceso dividir en pos inpares
                                if( procesos[3] == 0 ){
                                    int divIndex = 1;
                                    int anterior = 2;
                                    while( ! *fin )
                                    {                                        
                                        if( memoria[anterior] != 0 ){
                                            sem_wait( semaforos[0] );
                                            sem_wait( semaforos[1] );
                                            sem_wait( semaforos[2] );
                                            memoria[divIndex] = ( memoria[divIndex] / memoria[anterior] < _SC_INT_MIN ? rand() : memoria[divIndex] / memoria[anterior] );
                                            sem_post( semaforos[0] );
                                            sem_post( semaforos[1] );
                                            sem_post( semaforos[2] );
                                            sem_post( semaforos[3] );
                                            anterior = ( anterior + 2) % CANT_ELEMENTOS;
                                            divIndex = ( divIndex + 2 ) % CANT_ELEMENTOS;
                                        }
                                    }
                                }
                                else{
                                    cout<<"Este es el padre y debiera de mostrarse una vez"<<endl;
                                }
                            }
                }
        }
    
    if( procesos[0] > 0 && procesos[1] > 0 && procesos[2] > 0 && procesos[3] > 0){
        cout<<"Se estan ejecutando los 4 procesos hijos, por favor presione una techa para continuar....."<<endl;
        string f;
        cin>>f;
        *fin = true;
        waitpid( procesos[0], NULL, 0 );
        waitpid( procesos[1], NULL, 0 );
        waitpid( procesos[2], NULL, 0 );
        waitpid( procesos[3], NULL, 0 );
        sem_close( semaforos[0] );
        sem_close( semaforos[1] );
        sem_close( semaforos[2] );
        sem_close( semaforos[3] );
        *fin = false; //la vuelvo a dejar en su valor inicial
        munmap( fin, sizeof(bool) );
        cout<<"El padre libero todos los pids y semaforos, se muestra el vector final"<<endl;
        verMemoria( memoria );
        munmap( memoria, sizeof(int) * CANT_ELEMENTOS );
    }
    return 0;
}
