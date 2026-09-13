#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <stdio.h>

/* Функція my_handler - оброблювач сигналу SIGCHLD */ 
void my_handler(int nsig){
	int status;
	pid_t pid;
	/* Запит статусу процесу, що завершився, і одночасне одержання його ідентифікатора */ 
	if((pid = waitpid(-1, &status, 0)) < 0){
		/* Якщо виникла помилка - повідомлення про неї й продовження роботи */ 
		printf("Some error on waitpid errno = %d\n", errno);
	} else {
		/* Інакше аналіз статусу процесу, що завершився, */ 
		if ((status & 0xff) == 0) {
			/* Процес завершився з явним або неявним викликом функції exit() */ 
			printf("Process %dwasexitedwithstatus%d\n",pid, status>>8);
		}elseif ((status&0xff00) == 0){
			/*Процес був завершений за допомогою сигналу */ 
			printf("Process %d killed by signal %d %s\n", pid, status &0x7f,
			 (status & 0x80) ? "with core file" : "without core file");
		}
	}
}

int main(void){
	pid_t pid;
	/* Установка оброблювача для сигналу SIGCHLD */ 
	(void) signal(SIGCHLD, my_handler);
	/* Породження Сhild 1 */ 
	if((pid = fork()) < 0){
		printf("Can\'t fork child 1\n");
		exit(1);
	} elseif (pid==0){
		/*Child1-завершується з кодом 200 */
		exit(200);
	}

	/* Продовження процесу-батька - породжується Сhild 2 */ 
	if((pid = fork()) < 0){
		printf("Can\'t fork child 2\n");
		exit(1);
	} else if (pid == 0){
	/* Child 2 - циклиться, необхідно вбивати! */
		while(1);
	}
	/* Продовження процесу-батька - відхід у цикл */ 
	while(1);
	return 0;
}
