#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "TADList.h"
#include "TADDB.h"

void manipularBanco(BancoDados**Banco, ListEnc**Lista);
void ConstruirBanco(BancoDados**Banco, ListEnc**Lista);

int main(void)
{
    int flagB = 0, tecla;
    char comando[1500], diretorio[250];
    //Variaveis Locais
    ListEnc *Lista;
    BancoDados *Banco;
    Tabelas *tabelas;
    Campos *Campos;

    //Inicialização das Estruturas
    InitLista(&Lista);
    initDataBase(&Banco);

    do
    {
        if(flagB == 0)
        {
            fflush(stdin);
            printf("[ESC] - Sair\n");
            printf("[F12] - Abrir o arquivo \n");
            tecla = getch();
            if(tecla == 134 && flagB == 0)
            {
                clrscr();
                printf("Caminho do arquivo: ");
                fflush(stdin);
                gets(diretorio);

                if(strcmp(diretorio,""))
                {
                    CriarListaComandos(&Lista, diretorio);
                    ConstruirBanco(&Banco, &Lista);
                    printf("\nBanco criado com sucesso!\n");
                    flagB = 1;
                }
                else
                    printf("Falha ao abrir arquivo de criacao do banco!\n");
                Sleep(2000);
                clrscr();
            }
        }
        else
        {
            printf("Digite os comandos de DML/DQL ou pressione [ENTER] para sair!\n");
            printf("\nDigite o comando: \n> ");
            gets(comando);

            if(strcmp(comando,""))
            {
                CriarListaUsuario(&Lista, comando);
                manipularBanco(&Banco,&Lista);
            }
        }
    }while(tecla != 27);

    return 0;
}

void manipularBanco(BancoDados**Banco, ListEnc**Lista)
{
	char comando[100], nomeTab[50], coluna[50], valor[50], tabelas[20][50], colunas[20][50], valorFloat[10], titulo[110];
	int linhasAfetadas[1000], numLinha, numColunas = 0, numTabelas = 0, i, acabou =0, avancou = 0;
	Tabelas *tabLista[20]; //Vetor de ponteiro que guarda a posição das tabelas
	
	ListEnc *ListaColunas, *auxWhere;
	Campos *aux, *campo, *colAux;
	Dados *NovoDado, *auxDados, *antDado, *atualDado;
	Tabelas *tabAux;
	
	InitLista(&ListaColunas);
	
	while(*Lista != NULL)
	{
		ListDequeue(&*Lista, comando);
		if(!stricmp(comando, "INSERT"))
		{
			//consumir INTO
			ListDequeue(&*Lista, comando);
			if(!stricmp(comando, "INTO"))
			{
				ListDequeue(&*Lista, nomeTab);
				
				//consumiu (
				ListDequeue(&*Lista, comando);
				
				while(strcmp(comando, ")"))
				{
					ListDequeue(&*Lista, coluna);
					ListEnqueue(&ListaColunas,coluna);
					
					//Consumir ,
					ListDequeue(&*Lista, comando);
				}
				//Consumir VALUES
				ListDequeue(&*Lista, comando);
				ListDequeue(&*Lista, comando);
				
				while(ListaColunas != NULL)
				{
					ListDequeue(&ListaColunas, coluna);
					aux = buscaCampo(*Banco, nomeTab, coluna);					
					
					ListDequeue(&*Lista, valor);
					
					NovoDado = (Dados *)malloc(sizeof(Dados));
					NovoDado->prox = NULL;
					
					if(aux->tipo == 'I')
						NovoDado->valor.vInt = atoi(valor);
					else if(aux->tipo == 'N')
					{
						strcpy(valorFloat, valor);
						for(i = 0; i < 2; i++)
						{
							ListDequeue(&*Lista, valor);
							strcat(valorFloat, valor);
						}
						NovoDado->valor.vFloat = atof(valorFloat);
					}
					else
						strcpy(NovoDado->valor.vChar,valor);
				
					
					if(aux->pDados == NULL)
					{
						aux->pDados = NovoDado;
						aux->atual = NovoDado;
					}
					else
					{
						auxDados = aux->pDados;
						
						while(auxDados->prox != NULL)
							auxDados = auxDados->prox;
							
						auxDados->prox = NovoDado;
					}
					
					if(ListaColunas != NULL)
    					ListDequeue(&*Lista, comando);
				}
				
				printf("Dado(s) Inserido(s) com Sucesso!\n");	
				
				
			}
		}
		else if (!stricmp(comando, "UPDATE"))
		{
		    ListDequeue(&*Lista, nomeTab);
		
		    // Consome SET
		    ListDequeue(&*Lista, comando);
		
		    auxWhere = buscaWhere(&*Lista);
		
		    tabAux = buscaTabela(*Banco, nomeTab);
		
		    if (tabAux != NULL)
		    {
		        // Reiniciar todos os ponteiros atuais
		        colAux = tabAux->campos;
		
		        while (colAux != NULL)
		        {
		            colAux->atual = colAux->pDados;
		            colAux = colAux->prox;
		        }
		
		        numLinha = 0;
		        campo = tabAux->campos;
		
		        // Descobrir quais linhas serão alteradas
		        while (campo != NULL && campo->atual != NULL)
		        {
		            if (auxWhere == NULL || verificaCondicao(*Banco, nomeTab, auxWhere))
		                linhasAfetadas[numLinha] = 1;
		            else
		                linhasAfetadas[numLinha] = 0;
		
		            numLinha++;
		
		            // Avançar todos os ponteiros atuais juntos
		            colAux = tabAux->campos;
		
		            while (colAux != NULL)
		            {
		                if (colAux->atual != NULL)
		                    colAux->atual = colAux->atual->prox;
		
		                colAux = colAux->prox;
		            }
		
		            // NÃO avançar campo aqui
		            // O while termina quando campo->atual chegar em NULL
		        }
		    }
		
		    // Processar os campos do SET
		    while (*Lista != NULL)
		    {
		        // Consome coluna
		        ListDequeue(&*Lista, coluna);
		
		        // Localiza o campo
		        campo = buscaCampo(*Banco, nomeTab, coluna);
		
		        // Consome =
		        ListDequeue(&*Lista, comando);
		
		        // Consome valor
		        ListDequeue(&*Lista, valor);
		
		        // Se for NUMERIC
		        if (campo != NULL && campo->tipo == 'N')
		        {
		            strcpy(valorFloat, valor);
		            if (*Lista != NULL && !strcmp((*Lista)->token, "."))
		            {
		                // Consome .
		                ListDequeue(&*Lista, comando);
		
		                // Consome parte decimal
		                if (*Lista != NULL)
		                {
		                    ListDequeue(&*Lista, valor);
		                    strcat(valorFloat, ".");
		                    strcat(valorFloat, valor);
		                }
		            }
		        }
		
		        // Consome vírgula
		        if (*Lista != NULL && !strcmp((*Lista)->token, ","))
		            ListDequeue(&*Lista, comando);
		
		        // Reiniciar ponteiros
		        tabAux = buscaTabela(*Banco, nomeTab);
		
		        if (tabAux != NULL)
		        {
		            colAux = tabAux->campos;
		
		            while (colAux != NULL)
		            {
		                colAux->atual = colAux->pDados;
		                colAux = colAux->prox;
		            }
		        }
		
		        //Eecuperar novamente o campo que será alterado
		        campo = buscaCampo(*Banco, nomeTab, coluna);
		
		        numLinha = 0;
		
		        // Percorrer as linhas
		        while (campo != NULL && campo->atual != NULL)
		        {
		            if (linhasAfetadas[numLinha] == 1)
		            {
		                if (campo->tipo == 'I')
		                {
		                    campo->atual->valor.vInt = atoi(valor);
		                }
		                else if (campo->tipo == 'N')
		                {
		                    campo->atual->valor.vFloat = atof(valorFloat);
		                }
		                else
		                {
		                    strcpy(campo->atual->valor.vChar, valor);
		                }
		            }
		
		            // Avançar todos os ponteiros atuais juntos
		            if (tabAux != NULL)
		            {
		                colAux = tabAux->campos;
		
		                while (colAux != NULL)
		                {
		                    if (colAux->atual != NULL)
		                        colAux->atual = colAux->atual->prox;
		
		                    colAux = colAux->prox;
		                }
		            }
		
		            numLinha++;
		        }
		    }
		
		    // Reiniciar todos os ponteiros no final
		    tabAux = buscaTabela(*Banco, nomeTab);
		
		    if (tabAux != NULL)
		    {
		        colAux = tabAux->campos;
		
		        while (colAux != NULL)
		        {
		            colAux->atual = colAux->pDados;
		            colAux = colAux->prox;
		        }
		    }
		}
		else if (!stricmp(comando, "DELETE"))
		{
			//Consome FROM
			ListDequeue(&*Lista,comando);
			ListDequeue(&*Lista,nomeTab);
			
			auxWhere = buscaWhere(&*Lista);
			
			tabAux = buscaTabela(*Banco,nomeTab);
			
			colAux = tabAux->campos;
			while(colAux != NULL)
			{
				colAux->atual = colAux->pDados;
				colAux = colAux->prox;
			}
			
			numLinha = 0;
			campo = tabAux->campos; 
			
			while(campo != NULL && campo ->atual != NULL)
			{
				if(auxWhere == NULL || verificaCondicao(*Banco, nomeTab, auxWhere))
					linhasAfetadas[numLinha] = 1;
				else
					linhasAfetadas[numLinha] = 0;
						
				numLinha++;
				
				colAux = tabAux->campos;
				while (colAux != NULL) {
					if (colAux->atual != NULL) 
						colAux->atual = colAux->atual->prox;
					colAux = colAux->prox;
				}
			}
			
			colAux = tabAux->campos;
			while (colAux != NULL) 
			{
				numLinha = 0;
				antDado = NULL;
				atualDado = colAux->pDados;
				
				while (atualDado != NULL) 
				{
					if (linhasAfetadas[numLinha] == 1) 
					{
						if (antDado == NULL) {
							colAux->pDados = atualDado->prox;
							free(atualDado);
							atualDado = colAux->pDados;
						} 
						else 
						{
							antDado->prox = atualDado->prox;
							free(atualDado);
							atualDado = antDado->prox;
						}
					} 
					else 
					{
						antDado = atualDado;
						atualDado = atualDado->prox;
					}
					numLinha++;
				}
					
				colAux->atual = colAux->pDados;
					
				colAux = colAux->prox;
			}
		}
		else if(!stricmp(comando, "SELECT"))
		{	
			resetarPDados(&*Banco);
			if (!strcmp((*Lista)->token, "*"))
			{
			    auxWhere = buscaWhere(&*Lista);
			
			    // Consome * e FROM
			    ListDequeue(&*Lista, comando);
			    ListDequeue(&*Lista, comando);
			
			    // Guarda as tabelas do FROM
			    while (*Lista != NULL)
			    {
			        if (!strcmp((*Lista)->token, ","))
			            ListDequeue(&*Lista, comando);
			        else
			        {
			            ListDequeue(&*Lista, nomeTab);
			            tabLista[numTabelas++] = buscaTabela(*Banco, nomeTab);
			        }
			    }
			
				//CABEÇALHO
			    for (i = 0; i < numTabelas; i++)
				{
				    aux = tabLista[i]->campos;
				
				    while (aux != NULL)
				    {
				        printf("+----------------------");
				        aux = aux->prox;
				    }
				}
				printf("+\n");
				
				for (i = 0; i < numTabelas; i++)
				{
				    aux = tabLista[i]->campos;
				
				    while (aux != NULL)
				    {
				        sprintf(titulo, "%s.%s", tabLista[i]->nomeTab, aux->nomeCampo);
				        printf("| %-20s ", titulo);
				        aux = aux->prox;
				    }
				}
				printf("|\n");
				

				for (i = 0; i < numTabelas; i++)
				{
				    aux = tabLista[i]->campos;
				
				    while (aux != NULL)
				    {
				        printf("+----------------------");
				        aux = aux->prox;
				    }
				}
				printf("+\n");
				//FIM DO CABEÇALHO
			
			    acabou = 0;
				for (i = 0; i < numTabelas; i++)
				{
				    if (tabLista[i] == NULL || tabLista[i]->campos == NULL)
				    {
				        acabou = 1;
				    }
				    else
				    {
				        aux = tabLista[i]->campos;
				
				        while (aux != NULL)
				        {
				            if (aux->atual == NULL)
				                acabou = 1;
				
				            aux = aux->prox;
				        }
				    }
				}
			
			    while (!acabou)
			    {
		            if (auxWhere == NULL ||(numTabelas == 1 && verificaCondicao(*Banco, tabLista[0]->nomeTab, auxWhere)) ||(numTabelas > 1 && verificaCondicaoDuasTabs(*Banco, auxWhere)))
					{
					    for (i = 0; i < numTabelas; i++)
					    {
					        aux = tabLista[i]->campos;
					
					        while (aux != NULL)
					        {
					            if (aux->tipo == 'I')
					                printf("| %20d ", aux->atual->valor.vInt);
					            else if (aux->tipo == 'N')
					                printf("| %20.2f ", aux->atual->valor.vFloat);
					            else
					                printf("| %-20s ", aux->atual->valor.vChar);
					
					            aux = aux->prox;
					        }
					    }
					
					    printf("|\n");
					}
			
			        avancou = 0;
					for (i = numTabelas - 1; i >= 0 && !avancou && !acabou; i--)
					{
					    colAux = tabLista[i]->campos;
					
					    while (colAux != NULL)
					    {
					        if (colAux->atual != NULL)
					            colAux->atual = colAux->atual->prox;
					
					        colAux = colAux->prox;
					    }
					
					    if (tabLista[i]->campos->atual != NULL)
					    {
					        avancou = 1;
					    }
					    else if (i == 0)
					    {
					        acabou = 1;
					    }
					    else
					    {
					        colAux = tabLista[i]->campos;
					
					        while (colAux != NULL)
					        {
					            colAux->atual = colAux->pDados;
					            colAux = colAux->prox;
					        }
					    }
					}
			    }
			    for (i = 0; i < numTabelas; i++)
				{
				    aux = tabLista[i]->campos;
				
				    while (aux != NULL)
				    {
				        printf("+----------------------");
				        aux = aux->prox;
				    }
				}
				printf("+\n");
			
			    // Reinicia os ponteiros
			    for (i = 0; i < numTabelas; i++)
			    {
			        colAux = tabLista[i]->campos;
			        while (colAux != NULL)
			        {
			            colAux->atual = colAux->pDados;
			            colAux = colAux->prox;
			        }
			    }
			}
			else if(!strcmp((*Lista)->prox->token,"."))
			{			
				auxWhere = buscaWhere(&*Lista);

				while(stricmp((*Lista)->token, "FROM") != 0)
				{
					ListDequeue(&*Lista, tabelas[numColunas]);
					//Consome .
					ListDequeue(&*Lista, comando); 
					ListDequeue(&*Lista, colunas[numColunas++]);
					
					if(!strcmp((*Lista)->token, ","))
						ListDequeue(&*Lista, comando);
				}

				//Consome FROM
				ListDequeue(&*Lista, comando);

				//Le tabelas
				while(*Lista != NULL)
				{
					ListDequeue(&*Lista, nomeTab);
					tabLista[numTabelas++] = buscaTabela(*Banco, nomeTab);
					
					//Consome ,
					if(*Lista != NULL)
						ListDequeue(&*Lista, comando); 
				}

				//Posiciona primeiro registro
				colAux = tabLista[0]->campos;
				while(colAux != NULL)
				{
					colAux->atual = colAux->pDados;
					colAux = colAux->prox;
				}

				//Borda de cima
				for(i = 0; i < numColunas; i++)
				    printf("+----------------");
				
				printf("+\n");
				

				//Cabeçalho			
				for(i = 0; i < numColunas; i++)
				    printf("| %-14s ", colunas[i]);
				
				printf("|\n");
				
				for(i = 0; i < numColunas; i++)
				    printf("+----------------");
				
				printf("+\n");

				while(!acabou)
				{
					if(auxWhere == NULL || verificaCondicaoDuasTabs(*Banco, auxWhere))
					{
						for(i = 0; i < numColunas; i++)
						{
							campo = buscaCampo(*Banco, tabelas[i], colunas[i]);
							if(campo->tipo == 'I')
								printf("| %14d ", campo->atual->valor.vInt);
							else if(campo->tipo == 'N')
								printf("| %14.2f ", campo->atual->valor.vFloat);
							else
								printf("| %-14s ", campo->atual->valor.vChar);
						}
						printf("|\n");
					}

					avancou = 0;
					for (i = numTabelas - 1; i >= 0 && !avancou && !acabou; i--)
					{
					    colAux = tabLista[i]->campos;
					
					    while (colAux != NULL)
					    {
					        if (colAux->atual != NULL)
					            colAux->atual = colAux->atual->prox;
					
					        colAux = colAux->prox;
					    }
					
					    if (tabLista[i]->campos->atual != NULL)
					    {
					        avancou = 1;
					    }
					    else if (i == 0)
					    {
					        acabou = 1;
					    }
					    else
					    {
					        colAux = tabLista[i]->campos;
					
					        while (colAux != NULL)
					        {
					            colAux->atual = colAux->pDados;
					            colAux = colAux->prox;
					        }
					    }
					}
				}

				//Borda de baixo
				for(i = 0; i < numColunas; i++)
				    printf("+----------------");
				
				printf("+\n");
			}
			else
			{
			    auxWhere = buscaWhere(&*Lista);
			
			    // Guarda as colunas que serão exibidas
			    while(stricmp((*Lista)->token, "FROM") != 0)
			    {
			        ListDequeue(&*Lista, colunas[numColunas++]);
			
			        // Consome ,
			        if(*Lista != NULL && !strcmp((*Lista)->token, ","))
			            ListDequeue(&*Lista, comando);
			    }
			
			    // Consome FROM
			    ListDequeue(&*Lista, comando);
			
			    // Consome nome da tabela
			    ListDequeue(&*Lista, nomeTab);
			
			    tabAux = buscaTabela(*Banco, nomeTab);
			
			    if(tabAux != NULL)
			    {
			        // Posiciona todos os campos no primeiro registro
			        colAux = tabAux->campos;
			
			        while(colAux != NULL)
			        {
			            colAux->atual = colAux->pDados;
			            colAux = colAux->prox;
			        }
			
			        //Borda de cima
			        for(i = 0; i < numColunas; i++)
			            printf("+----------------");
			
			        printf("+\n");
			
			        /* =========================
			           CABEÇALHO
			           ========================= */
			
			        for(i = 0; i < numColunas; i++)
			            printf("| %-14s ", colunas[i]);
			
			        printf("|\n");
			
			        //Separação cabeçalho/dados
			
			        for(i = 0; i < numColunas; i++)
			            printf("+----------------");
			
			        printf("+\n");
			
			        //Dados
			
			        // Percorre os registros
			        while(tabAux->campos != NULL && tabAux->campos->atual != NULL)
			        {
			            // Verifica todas as condições do WHERE
			            if(auxWhere == NULL || verificaCondicao(*Banco, nomeTab, auxWhere))
			            {
			            	//printf("PASSOU!!!!!!!%d\n",verificaCondicao(*Banco, nomeTab, auxWhere)); APENAS PARA DEBUG
			                for(i = 0; i < numColunas; i++)
			                {
			                    campo = buscaCampo(*Banco, nomeTab, colunas[i]);
			
			                    if(campo->tipo == 'I')
			                        printf("| %14d ", campo->atual->valor.vInt);
			                    else if(campo->tipo == 'N')
			                        printf("| %14.2f ", campo->atual->valor.vFloat);
			                    else
			                        printf("| %-14s ", campo->atual->valor.vChar);
			                }
			
			                printf("|\n");
			            }
			
			            // Avança todos os campos juntos
			            colAux = tabAux->campos;
			
			            while(colAux != NULL)
			            {
			                if(colAux->atual != NULL)
			                    colAux->atual = colAux->atual->prox;
			
			                colAux = colAux->prox;
			            }
			        }
			        
			        //Borda de baixo
			        for(i = 0; i < numColunas; i++)
			            printf("+----------------");
			
			        printf("+\n");
			
			
			        // Reinicia os ponteiros atuais
			        colAux = tabAux->campos;
			
			        while(colAux != NULL)
			        {
			            colAux->atual = colAux->pDados;
			            colAux = colAux->prox;
			        }
			    }
			}
		}
	}
}
			




//comandos DDL(CREATE TABLE e ALTER TABLE - ALTER TABLE serve apenas para adicionar a
//				constraint de chave estrangeira)
void ConstruirBanco(BancoDados**Banco, ListEnc**Lista)
{
	int flagTabela, flagModificadores, consumir;
	char comando[50], nome[50], nomeTab[50], tipoCampo[50], tipo, nomeRef[50], coluna[50], tabRef[50];
	Campos regCampo, *aux, *auxCampo;
	
	while(*Lista != NULL)
	{
		ListDequeue(&*Lista, comando);
		if(!strcmp(comando, "CREATE"))
		{
			ListDequeue(&*Lista, comando);
			if(!strcmp(comando, "DATABASE"))
			{
				ListDequeue(&*Lista, nome);
				createDataBase(&*Banco, nome);
			}
			else if(!strcmp(comando, "TABLE"))
			{
				ListDequeue(&*Lista, nome);
				createTable(&*Banco, nome);
				strcpy(nomeTab,nome);
				
				ListDequeue(&*Lista, comando);
				if(!strcmp(comando, "("))
				{
					flagTabela = 1;
					while(flagTabela && *Lista != NULL)
					{
						ListDequeue(&*Lista, comando);
						
						if(!strcmp(comando, ")"))
						{
							flagTabela = 0;
						}
						else if(strcmp(comando, ",") != 0 && strcmp(comando, ";") != 0)
						{
							if(!strcmp(comando, "CONSTRAINT"))
						    {
						        //Ler CONSTRAINT
						        ListDequeue(&*Lista, comando);
						        //nomeRef não ta servindo pra nada
						        strcpy(nomeRef,comando);
						        
						        ListDequeue(&*Lista, comando);
						        
						        if(!strcmp(comando, "PRIMARY"))
						        {
						        	//Consome o KEY
						        	ListDequeue(&*Lista, comando);
						        	
						        	//Consome (
						        	ListDequeue(&*Lista, comando);
						        	
						        	ListDequeue(&*Lista, coluna);
						        	
						        	while(strcmp(coluna,")") != 0)
						        	{
						        		if(strcmp(coluna,",") != 0)
						        		{
							        		aux = buscaCampo(*Banco, nomeTab, coluna);
							        		aux->PK = 'S';	
						        		}

						        		ListDequeue(&*Lista, coluna);
						        	}
						        }				        	    
						    }
						    else
						    {
						    	regCampo.tamanho = 0;
						    	regCampo.notNull = 'N';
						    	regCampo.PK = 'N';
						    	strcpy(regCampo.nomeCampo, comando);
						    	
						    	ListDequeue(&*Lista, tipoCampo);
						    	
						    	flagModificadores = 1;
								while(flagModificadores && *Lista != NULL)
								{
									ListDequeue(&*Lista, comando);
									
									if(!strcmp(comando, ","))
										flagModificadores = 0;
									else if(!strcmp(comando, ")"))
									{
										flagModificadores = 0; 
										flagTabela = 0;
									}
									else if(!strcmp(comando, "("))
									{
									    ListDequeue(&*Lista, comando);
									    regCampo.tamanho = atoi(comando);
									    //regCampo.escala = 0;
									    consumir = 1;
									    while(consumir && *Lista != NULL)
									    {
									        ListDequeue(&*Lista, comando);
									        
									        if(!strcmp(comando, ","))
									            ListDequeue(&*Lista, comando);
									            //regCampo.escala = atoi(comando);
									        else if(!strcmp(comando, ")"))
									            consumir = 0;
									    }
									}
									else if(!strcmp(comando, "NOT"))
									{
										ListDequeue(&*Lista, comando); //Serve só para consumir o table
										regCampo.notNull = 'S';
									}
								}
								
								if(!strcmp(tipoCampo, "INTEGER")) regCampo.tipo = 'I';
								else if(!strcmp(tipoCampo, "CHARACTER")) regCampo.tipo = 'C';
								else if(!strcmp(tipoCampo, "NUMERIC")) regCampo.tipo = 'N';
								else if(!strcmp(tipoCampo, "DATE")) regCampo.tipo = 'D';
								
								addCampo(&*Banco, nome, regCampo);
							}	
						}	
					}
				}
			}
		}
		else if(!strcmp(comando, "ALTER"))
		{
			ListDequeue(&*Lista, comando);
			if(!strcmp(comando, "TABLE"))
			{
				ListDequeue(&*Lista, nomeTab);
				
				//Consome o ADD
				ListDequeue(&*Lista, comando);
						        	
				//Consome CONSTRAINT
				ListDequeue(&*Lista, comando);
					
				ListDequeue(&*Lista, nomeRef);			
				
				//Consome Foreign
				ListDequeue(&*Lista, comando);	
				
				//Consome KEY
				ListDequeue(&*Lista, comando);
				
				//Consome (
				ListDequeue(&*Lista, comando);
				
				ListDequeue(&*Lista, coluna);			        	
						        	
				aux = buscaCampo(*Banco, nomeTab, coluna);
						        	
				//Consome )
				ListDequeue(&*Lista, comando);
						        	
				//Consome REFERENCES
				ListDequeue(&*Lista, comando);
						        	
				ListDequeue(&*Lista, tabRef);					        
						        	
				//Consome (
				ListDequeue(&*Lista, comando);
						        	
				ListDequeue(&*Lista, coluna);	
														        	
				auxCampo = buscaCampo(*Banco, tabRef, coluna);
				
				//Consome )
				ListDequeue(&*Lista, comando);
						        	
				aux->FK = auxCampo;
			}
		}
	}
}



