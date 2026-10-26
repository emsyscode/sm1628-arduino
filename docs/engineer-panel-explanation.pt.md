# Engineer's original explanation — 2026-10-26

Preserved verbatim as relayed by the creator. Numeric positions count from
the right, confirmed separately by the creator. These are panel group labels;
correlation with raw RAM columns remains to be recorded.

1-O SM1628 tem a capacidade de controlar até 7 digitos com maximo de 10 segmentos, mas obriga a alguns truques, inicialmente não está desenhado para isso.
2-O fabricante do painel efectuou a troca dos pinos dos segmentos com as Grelhas/Digitos, esta troca passa a permitir controlar 10 digitos com 7 segmentos havendo a necessidade de efectuar a troca do display.  Cathode comum tem de passar a Anode comum!
3-O painel já se encontra construído e montado, assim a ordem que se impõe é obrigatoriamente respeitar as ligações de hardware!
4-A ordem a estabelecer com a tabela interna do SM1628 tem de respeitar a ordem dos digitos no display LED.
5-A distribuição do digitos no display é: 
Grid 1 Representa os symbolos "return", ":" left, ":" right, "Dolby", "DTS",
Grid 2. 1º digito á direita
Grid 3, 2º digito à direita
Grid 4, 3º digito
Grid 5, 4º digito
Grid 6, 7º digito
Grid 7, 5º digito
Grid 8, 6º digito
Grid 9, Symbolo Wheels
Grid 10, Symbols: DVD, VCD, MP3, PBC, Play, Pause and the last symbol of Wheels
6-A aplicação destas considerações resulta na conversão da tabela de memória interna que passa de 2 bytes horizontais para representar um digito na Dig1/Grid1 para uma coluna vertical de bits, Isso obriga ao envio sempre da totalidade dos 14 bytes para preencher qualquer digito!
