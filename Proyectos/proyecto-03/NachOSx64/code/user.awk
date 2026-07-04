{ print "curl \"https://os.ecci.ucr.ac.cr/check/?curso=" $5 "&year=" substr($6,0,4) "&ciclo=" substr($6,6,1) "&carnet=" $8 "&grupo=" substr($7,2,1) "\"" }
