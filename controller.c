// ---------- Máquina de estados -----------

// 01101111 01101001 00100000 01110000 01110010 01101111 01100110 01100101 01110011 01110011 01101111 01110010 

#include "IO.c"

#define DEBUG

// Tipos de dados

typedef enum
{
	PARADO,	 // 0
	OPERAR,	 // 1
	A_PARAR, // 2
} modos;

typedef enum
{
	TAPETES_ON,	 // 0
	ANDAR_VERDE, // 1
	EMPURRAR_P,	 // 2
	RETRAIR_P,	 // 3
	ANDAR_AZUL,	 // 4
	ANDAR_AZUL2, // 5
	BUFFER,		 // 6
	PARAR_T,	 // 7
} separacao;

typedef enum {
	WAIT_OFF, // 0
	WAIT_ON,  // 1
} luz_do_a_parar;

// timer block typedef
typedef struct{
	bool on;
	uint64_t time;
} timerBlock;

// Funções
void initME();
void update_timers();
//void start_timer(timerBlock *t);
//void stop_timer(timerBlock *t);

// Estado atual da máquina
modos currentState_modos = PARADO;
separacao currentState_separacao = PARAR_T;
luz_do_a_parar currentState_luz = WAIT_OFF;

// Tempo de ciclo
uint64_t scan_time = 100; // 100ms

// timers
uint64_t start_time = 0, end_time = 0, cycle_time = 0;
timerBlock timer1;
timerBlock timer2;

// variáveis de flancos
bool FE_START = false; // falling edge do botao start
int startPreviousState = 0;

bool RE_STOP = false; // rising edge do botao stop
int stopPreviousState = 0;

bool RE_SPR = false;
int sprPreviousState = 0;

bool RE_SPE = false;
int spePreviousState = 0;

bool FE_STR2 = false;
int str2PreviousState = 0;

bool FE_ST2 = false;
int st2PreviousState = 0;

bool FE_ST3 = false;
int st3PreviousState = 0;

// Atualiza timers
void update_timers()
{
	// Calcula o tempo de ciclo
	end_time = get_time();
	if (start_time == 0)
		cycle_time = 0;
	else
		cycle_time = end_time - start_time; // o fim do ciclo atual é o inicio do próximo start_time = end_time;

	// Atualiza temporizadores
	if (timer1.on)
		timer1.time = timer1.time + cycle_time;
	if (timer2.on)
		timer2.time = timer2.time + cycle_time;
}

void start_timer(timerBlock *t)
{
	t->on = true;
	t->time = 0;
}

void stop_timer(timerBlock *t)
{
	t->on = false;
	t->time = 0;
	// t->time pode ser comentado
	// se for importante guardar
	// o valor do temporizador
}

// Inicializa a ME
void init_ME()
{
	LSTART = 0;
	LSTOP = 0;
	LWAIT = 0;
	E1 = 0;
	E2 = 0;
	T1A = 0;
	T2A = 0;
	T3A = 0;
	T4A = 0;
	PE1 = 0;
	PR1 = 0;
	PE2 = 0;
	PR2 = 0;
	AZUIS = 0;
	VERDES = 0;
}

// Código principal
int main()
{
	init_ME(); // Inicialização da ME

	// Ciclo de execução
	while (1)
	{

#ifdef DEBUG
		printf("\n*** Inicio do Ciclo ***\n");
#endif

		// Leitura das entradas
		read_inputs();
		update_timers();

		// detetecao de flancos
		FE_START = (startPreviousState && !START) ? true : false;
		startPreviousState = START;
		RE_STOP = (!stopPreviousState && STOP) ? true : false;
		stopPreviousState = STOP;
		RE_SPR = (!sprPreviousState && SPR1) ? true : false;
		sprPreviousState = SPR1;
		RE_SPE = (!spePreviousState && SPE1) ? true : false;
		spePreviousState = SPE1;
		FE_STR2 = (str2PreviousState && !STR2) ? true : false;
		str2PreviousState = STR2;
		FE_ST2 = (st2PreviousState && !ST2) ? true : false;
		st2PreviousState = ST2;
		FE_ST3 = (st2PreviousState && !ST2) ? true : false;
		st2PreviousState = ST2;

		update_timers();

		// Transição entre estados MODOS
		switch (currentState_modos)
		{

		case PARADO:
			// Testa transição Parado -> Operar
			#ifdef DEBUG
				printf("\nmodos.PARADO\n");
			#endif
			if (FE_START == true)
				// Próximo estado
				currentState_modos = OPERAR;

			break;

		case OPERAR:
			// Testa transição OPERAR -> A_PARAR
			#ifdef DEBUG
				printf("\nmodos.OPERAR\n");
			#endif
			if (RE_STOP == true)
				currentState_modos = A_PARAR;

			break;

		case A_PARAR:
			// Testa transição A_PARAR -> PARADO
			#ifdef DEBUG
				printf("\nmodos.A_PARAR\n");
			#endif
			if (currentState_separacao == PARAR_T)
				currentState_modos = PARADO;

			break;

		default:
			break;
		}

		// Transição entre estados separacao
		//TODO é necessário refazer a implementação do timer
		switch (currentState_separacao)
		{
		case TAPETES_ON:
		#ifdef DEBUG
				printf("\nseparacao.TAPETES_ON\n");
		#endif
			if (!timer1.on)
				start_timer(&timer1);
			if (SV1 == 4)
				currentState_separacao = ANDAR_VERDE;
			else if (SV1 == 1)
				currentState_separacao = ANDAR_AZUL;
			else if (currentState_modos == A_PARAR && timer1.time >= 10)
				currentState_separacao = BUFFER;
			else if (currentState_separacao != TAPETES_ON)
				stop_timer(&timer1);

			break;

		case ANDAR_VERDE:
		#ifdef DEBUG
				printf("\nseparacao.ANDAR_VERDE\n");
		#endif
			if (FE_STR2 == true)
				currentState_separacao = EMPURRAR_P;
			break;

		case EMPURRAR_P:
		#ifdef DEBUG
				printf("\nseparacao.EMPURRAR_P\n");
		#endif
			if (RE_SPE == true)
				currentState_separacao = RETRAIR_P;
			break;

		case RETRAIR_P:
		#ifdef DEBUG
				printf("\nseparacao.RETRAIR_P\n");
		#endif
			if (RE_SPR == true)
				currentState_separacao = TAPETES_ON;
			break;
		case ANDAR_AZUL:
		#ifdef DEBUG
				printf("\nseparacao.ANDAR_AZUL\n");
		#endif
			if (FE_STR2 == true)
				currentState_separacao = ANDAR_AZUL2;
			break;

		case ANDAR_AZUL2:
		#ifdef DEBUG
				printf("\nseparacao.ANDAR_AZUL2\n");
		#endif
			if (FE_ST2 == true)
				currentState_separacao = TAPETES_ON;
			break;

		case BUFFER:
		#ifdef DEBUG
				printf("\nseparacao.BUFFER\n");
		#endif
			if (!timer1.on)
				start_timer(&timer1);
			if (timer1.time >=15)
				currentState_separacao = PARAR_T;
			break;

		case PARAR_T:
		#ifdef DEBUG
				printf("\nseparacao.PARAR_T\n");
		#endif
			if (currentState_modos == OPERAR)
				currentState_separacao = TAPETES_ON;
			break;

		default:
			break;
		}

		// Transição entre estados LUZ_DO_A_PARAR
		switch (currentState_luz)
		{
		case WAIT_OFF:
		#ifdef DEBUG
				printf("\ncurrent_State_luz.WAIT_OFF\n");
		#endif
			if(!timer2.on)
				start_timer(&timer2);
			if (A_PARAR == true && timer2.time >=1){
				currentState_luz = WAIT_ON;
				stop_timer(&timer1);
			}
			break;

		case WAIT_ON: // scatman
		#ifdef DEBUG
				printf("\ncurrent_State_luz.WAIT_ON\n");
		#endif
			if(!timer2.on)
				start_timer(&timer2);
			if (timer2.time >= 1){
				currentState_luz = WAIT_OFF;
				stop_timer(&timer2);
			}
			break;

		default:
			break;
		}
		// Atualiza saídas
		// Saídas booleanas
		//Saidas Modos
		if (currentState_modos == PARADO){
			#ifdef DEBUG
				printf("\nsaidas_modos.PARADO\n");
			#endif
			E1 = false;
			LSTART = false;
			LSTOP = true;
			
		}
		else if (currentState_modos == OPERAR){
			#ifdef DEBUG
				printf("\nsaidas_modos.OPERAR\n");
			#endif
			LSTOP = false;
			LSTART = true;
			E1 = true;
		}
		else if (currentState_modos == A_PARAR){
			#ifdef DEBUG
				printf("\nsaidas_modos.A_PARAR\n");
			#endif
			LSTOP = false;
			LSTART = false;
			E1 = false;
		}

		//Saídas Separacao
		if(currentState_separacao == TAPETES_ON) {
			#ifdef DEBUG
				printf("\nsaidas_separacao.TAPETES_ON\n");
			#endif
            PR1 = 0;	
            T1A = 1;
            T2A = 1;
            T3A = 1;
		}

		else if(currentState_separacao == ANDAR_VERDE) {
			#ifdef DEBUG
				printf("\nsaidas_separacao.ANDAR_VERDE\n");
			#endif
			T1A = 1;
			T2A = 1;
			T3A = 1;
		}

		else if (currentState_separacao == EMPURRAR_P){
			#ifdef DEBUG
				printf("\nsaidas_separacao.EMPURRAR_P\n");
			#endif
			T1A = 0;
			T2A = 0;
			T3A = 0;
			
			PE1 = 1;
		}		
		else if(currentState_separacao == RETRAIR_P){
			#ifdef DEBUG
				printf("\nsaidas_separacao.RETRAIR_P\n");
			#endif
			PE1 = 0;
			PR1 = 1;
		}
		else if(currentState_separacao == ANDAR_AZUL){
			#ifdef DEBUG
				printf("\nsaidas_separacao.ANDARA_AZUL\n");
			#endif
			T1A = 1;
			T2A = 1;
			T3A = 1;
		}
		else if(currentState_separacao == ANDAR_AZUL2){
			#ifdef DEBUG
				printf("\nsaidas_separacao.ANDAR_AZUL2\n");
			#endif
			T1A = 0;
			T2A = 1;
			T3A = 1;
		}
		else if(currentState_separacao == BUFFER){
			#ifdef DEBUG
				printf("\nsaidas_separacao.BUFFER\n");
			#endif
			T1A = 0;
			T2A = 1;
			T3A = 1;
		}
		else if(currentState_separacao == PARAR_T){
			#ifdef DEBUG
				printf("\nsaidas_separacao.PARAR_T\n");
			#endif
			T1A = 0;
			T2A = 0;
			T3A = 0;
		}

		if (currentState_luz == WAIT_ON){
			#ifdef DEBUG
				printf("\ncurrent_state_luz.WAIT_ON\n");
			#endif
			LWAIT = 1;
		} else {
			LWAIT = 0;
		}
		
		// Saidas Contador Azuis e Verdes
		if(FE_ST2){
			#ifdef DEBUG
				printf("\nconta.azuis\n");
			#endif
			AZUIS++;
		}
		if(FE_ST3){
			#ifdef DEBUG
				printf("\nconta.verdes\n");
			#endif
			VERDES++;
		}
		if(FE_START){
			AZUIS = 0;
			VERDES = 0;
		}


		// Escrita nas saídas
		write_outputs();

		// Aguarda pelo próximo ciclo
		sleep_abs(scan_time);

	} // end loop

} // end main
