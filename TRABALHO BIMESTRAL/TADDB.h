#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/* STRUCT DE DADOS */
typedef struct dados {
    union {
        int vInt;         // Para INTEGER ('I')
        float vFloat;     // Para NUMERIC ('N')
        char vChar[50];   // Para CHARACTER ('C') e DATE ('D')
    } valor;
    
    struct dados *prox;
} Dados;

/* STRUCT DE CAMPOS (Colunas) */
typedef struct campos {
	Dados *atual, *pDados;
	char nomeCampo[50], tipo, PK, notNull; //notNull == 'S' ou 'N'
	int tamanho; //Tamanho maximo da variavel não está sendo tratado
	struct campos *FK, *prox;
} Campos;

/* STRUCT DE TABELAS */
typedef struct tabelas {
	char nomeTab[50]; 
	Campos *campos;
	struct tabelas *ant, *prox;
} Tabelas;

/* STRUCT DO BANCO DE DADOS */
typedef struct bancoDados {
	char nome[50];
	Tabelas *tabelas;
} BancoDados;

void initDataBase(BancoDados**banco)
{
	*banco = NULL;
}

char isNullDB(BancoDados *banco)
{
	return banco == NULL;
}

void createDataBase(BancoDados**banco, char nome[50])
{
	BancoDados *novo;
	
	if(isNullDB(*banco))
	{
		novo = (BancoDados *)malloc(sizeof(BancoDados));
		strcpy(novo->nome, nome);
		novo->tabelas = NULL;
		
		*banco = novo;
	}
}

char isNullT(Tabelas *tabela)
{
	return tabela == NULL;
}

void createTable(BancoDados **banco, char nome[50])
{
    Tabelas *nova, *aux;

    if (!isNullDB(*banco))
    {
        nova = (Tabelas *) malloc(sizeof(Tabelas));
        strcpy(nova->nomeTab, nome);
        nova->campos = NULL;
        nova->ant = NULL;
        nova->prox = NULL;

        if (isNullT((*banco)->tabelas))
            (*banco)->tabelas = nova;
        else
        {
            aux = (*banco)->tabelas;

            while (aux->prox != NULL)
                aux = aux->prox;

            nova->ant = aux;
            aux->prox = nova;
        }
    }
}

void addCampo(BancoDados **banco, char nomeTabela[50], Campos regCampo)
{
    Tabelas *aux;
    Campos *novo, *auxCampo;
    
    if (!isNullDB(*banco))
    {
        aux = (*banco)->tabelas;
        
        while (aux != NULL && strcmp(aux->nomeTab, nomeTabela) != 0)
            aux = aux->prox;
        
        if (aux != NULL)
        {
            novo = (Campos *)malloc(sizeof(Campos));
            
            strcpy(novo->nomeCampo, regCampo.nomeCampo);
            novo->tipo = regCampo.tipo;
            novo->tamanho = regCampo.tamanho;
            novo->PK = regCampo.PK;
            novo->notNull = regCampo.notNull;
            
            novo->atual = NULL;
            novo->pDados = NULL;
            novo->FK = NULL;
            novo->prox = NULL;
            
            if (aux->campos == NULL)
                aux->campos = novo;
            else
            {
                auxCampo = aux->campos;
                while (auxCampo->prox != NULL)
                {
                    auxCampo = auxCampo->prox;
                }
                
                auxCampo->prox = novo;
            }
        }
        else
        {
            printf("Erro: Tabela '%s' nao encontrada no banco!\n", nomeTabela);
        }
    }
}

Campos* buscaCampo(BancoDados *banco, char nomeTabela[50], char nomeColuna[50])
{
	Tabelas *aux;
    Campos *auxCampo;
	
	if(!isNullDB(banco))
	{
		aux = banco->tabelas;
		
		while(aux != NULL && stricmp(aux->nomeTab, nomeTabela) != 0)
			aux = aux->prox;
		
		if(aux != NULL)
		{
			auxCampo = aux->campos;
			while(auxCampo != NULL && stricmp(auxCampo->nomeCampo, nomeColuna) != 0)
				auxCampo = auxCampo->prox;
		}
		
		return auxCampo;
	}
	
}

Tabelas* buscaTabela(BancoDados *banco, char nomeTabela[50])
{
	Tabelas *aux;
	
	if(!isNullDB(banco))
	{
		aux = banco->tabelas;
		
		while(aux != NULL && stricmp(aux->nomeTab, nomeTabela) != 0)
			aux = aux->prox;
		
		return aux;
	}
}

int verificaCondicao(BancoDados* Banco, char nomeTab[50], ListEnc* Lista)
{
	int condicaoAtendida = 1;
   	char condicao1[50], condicao2[50],condicao3[50], operador[10];
   	ListEnc *aux;    
	Campos *campoCondicao;
   	
   	aux = Lista;
   	
   	while(aux != NULL && strcmp(aux->token,";") != 0)
   	{
   		strcpy(condicao1, aux->token);
   		aux = aux->prox;
   		 
   		strcpy(operador, aux->token);
   		aux = aux->prox;
   		 
   		if (!strcmp(operador, "BETWEEN"))
		{
		    strcpy(condicao2, aux->token);
		    aux = aux->prox;
		
		    // Junta a parte decimal do limite inferior, se houver
		    if (aux != NULL && !strcmp(aux->token, "."))
		    {
		        strcat(condicao2, ".");
		        aux = aux->prox;
		
		        if (aux != NULL)
		        {
		            strcat(condicao2, aux->token);
		            aux = aux->prox;
		        }
		    }
		
		    // Consome o AND entre os limites do BETWEEN
		    if (aux != NULL && !stricmp(aux->token, "AND"))
		        aux = aux->prox;
		
		    if (aux != NULL)
		    {
		        strcpy(condicao3, aux->token);
		        aux = aux->prox;
		
		        // Junta a parte decimal do limite superior, se houver
		        if (aux != NULL && !strcmp(aux->token, "."))
		        {
		            strcat(condicao3, ".");
		            aux = aux->prox;
		
		            if (aux != NULL)
		            {
		                strcat(condicao3, aux->token);
		                aux = aux->prox;
		            }
		        }
		    }
		}
		else
		{
		    strcpy(condicao2, aux->token);
		    aux = aux->prox;
		
		    // Junta a parte decimal do valor comparado, se houver
		    if (aux != NULL && !strcmp(aux->token, "."))
		    {
		        strcat(condicao2, ".");
		        aux = aux->prox;
		
		        if (aux != NULL)
		        {
		            strcat(condicao2, aux->token);
		            aux = aux->prox;
		        }
		    }
		}
   		
   		campoCondicao = buscaCampo(Banco, nomeTab, condicao1);
   		
   		if(campoCondicao->tipo == 'I')
   		{
   			if(!strcmp(operador, "="))
			{
			    if(campoCondicao->atual->valor.vInt != atoi(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, ">"))
			{
			    if(campoCondicao->atual->valor.vInt <= atoi(condicao2))
			    	condicaoAtendida = 0;
			    	
			    
			}
			
			else if(!strcmp(operador, ">="))
			{
				if(campoCondicao->atual->valor.vInt < atoi(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, "<"))
			{
			    if(campoCondicao->atual->valor.vInt >= atoi(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, "<="))
			{
				if(campoCondicao->atual->valor.vInt > atoi(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, "!="))
			{
				if(campoCondicao->atual->valor.vInt == atoi(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if (!strcmp(operador, "BETWEEN"))
			{
				if(campoCondicao->atual->valor.vInt < atoi(condicao2) || campoCondicao->atual->valor.vInt > atoi(condicao3))
					condicaoAtendida = 0;
			}
   		}
   		else if(campoCondicao->tipo == 'N')
   		{
   			if(!strcmp(operador, "="))
			{
			    if(campoCondicao->atual->valor.vFloat != atof(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, ">"))
			{
			    if(campoCondicao->atual->valor.vFloat <= atof(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, ">="))
			{
			    if(campoCondicao->atual->valor.vFloat < atof(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, "<"))
			{
			    if(campoCondicao->atual->valor.vFloat >= atof(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, "<="))
			{
			    if(campoCondicao->atual->valor.vFloat > atof(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, "!="))
			{
			    if(campoCondicao->atual->valor.vFloat == atof(condicao2))
			        condicaoAtendida = 0;
			}
			
			else if (!strcmp(operador, "BETWEEN"))
			{
				if(campoCondicao->atual->valor.vFloat < atof(condicao2) || campoCondicao->atual->valor.vFloat > atof(condicao3))
					condicaoAtendida = 0;
			}
   		}
   		else
   		{
   			if(!strcmp(operador, "="))
			{
			    if(strcmp(campoCondicao->atual->valor.vChar,condicao2) != 0)
			        condicaoAtendida = 0;
			}
			
			else if(!strcmp(operador, "!="))
			{
			    if(strcmp(campoCondicao->atual->valor.vChar,condicao2) == 0)
			        condicaoAtendida = 0;
			}
   		}
   		
   		if(aux != NULL &&(!strcmp(aux->token, "AND")))
            aux = aux->prox;
        
   	}
   	
   	return condicaoAtendida;
}

int verificaCondicaoDuasTabs(BancoDados* Banco, ListEnc* Lista)
{
    int condicaoAtendida = 1;
    char tab1[50], col1[50], operador[10], tab2[50], col2[50];
    ListEnc *aux;    
    Campos *campo1, *campo2;
    
    aux = Lista;
    
    while(aux != NULL && strcmp(aux->token,";") != 0)
    {
        strcpy(tab1, aux->token);
        aux = aux->prox;
        //Consome .
        aux = aux->prox; 
        strcpy(col1, aux->token);
        aux = aux->prox;
        
        strcpy(operador, aux->token);
        aux = aux->prox;
        
        strcpy(tab2, aux->token);
        aux = aux->prox;
        //Consome .
        aux = aux->prox;
        strcpy(col2, aux->token);
        aux = aux->prox;
        
        campo1 = buscaCampo(Banco, tab1, col1);
        campo2 = buscaCampo(Banco, tab2, col2);
        
        if (campo1 != NULL && campo2 != NULL && campo1->atual != NULL && campo2->atual != NULL)
        {
            if(campo1->tipo == 'I')
            {
                if(!strcmp(operador, "="))
                {
                    if(campo1->atual->valor.vInt != campo2->atual->valor.vInt)
                        condicaoAtendida = 0;
                }
            }
            else if(campo1->tipo == 'N')
            {
                if(!strcmp(operador, "="))
                {
                    if(campo1->atual->valor.vFloat != campo2->atual->valor.vFloat)
                        condicaoAtendida = 0;
                }
            }
            else
            {
                if(!strcmp(operador, "="))
                {
                    if(strcmp(campo1->atual->valor.vChar, campo2->atual->valor.vChar) != 0)
                        condicaoAtendida = 0;
                }
            }
        }
        else
        {
            condicaoAtendida = 0; /* Falha se algum campo estiver vazio */
        }
        
        // Consome AND
        if(aux != NULL && (!strcmp(aux->token, "AND")))
            aux = aux->prox;
    }
    
    return condicaoAtendida;
}

void resetarPDados(BancoDados* *Banco)
{
	Tabelas *pTabelas;
	Campos * pCampos;
	
	pTabelas = (*Banco)->tabelas;
	
	while(pTabelas != NULL)
	{
		pCampos = pTabelas->campos;
		
		while(pCampos != NULL)
		{
			if(pCampos->pDados != NULL)
				pCampos->atual = pCampos->pDados;
			pCampos = pCampos->prox;
		}
		pTabelas = pTabelas->prox;
	}
}


