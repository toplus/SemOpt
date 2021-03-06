/**
 * @file 		AF.cpp
 * @class 		AF
 * @brief 		Class encompassing an Argumentation Framework
 * @author 		Mauro Vallati <m.vallati@hud.ac.uk>
 * @copyright	GNU Public License v2
 */


#include "AF.h"

/**
 * @brief 	Simple constructor
 */
AF::AF()
{
	this->arguments = new SetArguments();
	//this->raw_attacks = map<int, int >();
	//this->attacks = map<Argument *, SetArguments *>();
	//this->attackers = map<Argument *, SetArguments *>();
}


AF::~AF()
{
	// TODO Auto-generated destructor stub
}

/**
 * @brief 		Method for parsing a ASPARTIX compliant file
 * @details		It uses part of the code from Samer Nofal's, University of Liverpool
 * @param[in] file A string representing the (relative/absolute) path to the ASPARTIX compliant file
 * @retval bool It returns `True` if it can correctly parse the file, `False` otherwise
 */
bool AF::readFile(string file)
{
	string inLine;
	ifstream infile;
	infile.open(file.c_str());
	if (!infile.good())
		return false;
	infile >> inLine;
	while (inLine.find("arg") != string::npos && !infile.eof())
	{
		this->arguments->add_Argument(new Argument(
				inLine.substr(4, inLine.find_last_of(")") - 4),
				this->numArgs(),
				this
				));
		infile >> inLine;
	}

	while (!infile.eof())
	{
		if (inLine.find("att") != string::npos)
		{
			Argument *source = this->getArgumentByName(
					(inLine.substr(4,
							inLine.find_last_of(",") - 4)));
			Argument *target = this->getArgumentByName(
					(inLine.substr(inLine.find_last_of(",") + 1,
							inLine.find_last_of(")")
									- inLine.find_last_of(",")
									- 1)));

			source->add_attacks(target);
			target->add_attackers(source);
//			this->raw_attacks.push_back(
//					pair<int, int>(
//							this->argNameToCode(
//									(inLine.substr(4,
//											inLine.find_last_of(",") - 4))),
//							this->argNameToCode(
//									(inLine.substr(inLine.find_last_of(",") + 1,
//											inLine.find_last_of(")")
//													- inLine.find_last_of(",")
//													- 1)))));
		}
		infile >> inLine;
	}
	infile.close();
	return true;
}

/**
 * @brief		This method returns the pointer to the Argument whose name is given as parameter
 * @param[in] name	 The name of the argument
 * @retval Argument* The pointer to the Argument object, or NULL if not found
 */
Argument *AF::getArgumentByName(string name)
{
	return this->arguments->getArgumentByName(name);
}

/**
 * @brief 	Returns the number of arguments
 * @retval int
 */
int AF::numArgs()
{
	return this->arguments->cardinality();
}


/**
 * @brief		This method returns the pointer to the Argument whose identification number is given as parameter
 * @param[in] num	 The name of the argument
 * @retval Argument* The pointer to the Argument object, or NULL if not found
 */
Argument *AF::getArgumentByNumber(int num)
{
	return this->arguments->getArgumentByNumber(num);
}

/**
 * @brief 	Begin of the iterator for the set of arguments
 * @retval SetArgumentsIterator An iterator pointing at the first of the elements of the set of arguments
 */
SetArgumentsIterator AF::begin()
{
	return this->arguments->begin();
}

/**
 * @brief 	End of the iterator for the set of arguments
 * @retval SetArgumentsIterator An iterator pointing at the last of the elements of the set of arguments
 */
SetArgumentsIterator AF::end()
{
	return this->arguments->end();
}

/**
 * @brief	Returns a pointer to the set of arguments
 * @retval SetArguments*
 */
SetArguments *AF::get_arguments() const
{
	return this->arguments;
}

/**
 * @brief 		Create a new AF, with a copy of SetArguments of the AF in input
 * @param[in] 	AF to copy
 */
AF::AF(const AF& gamma){
	this->arguments=new SetArguments(*gamma.get_arguments());
}


/**
 * @brief 		Method for reduce an AF on a Set of Arguments
 * @details		The method reduce the SetArguments of this AF, considering only the Arguments which are in the SetArguments I.
				The suspended attacks are removed from the reduced AF
 * @param[in] 	The SetArguments which is used to reduce the AF
 * @retval 		Returns a new AF, the reduced one
 */
AF AF::reduceAF(SetArguments I){
	int index=0;
	AF *gamma_reduced = new AF();
	gamma_reduced->arguments = new SetArguments();

	SetArgumentsIterator yt = gamma_reduced->arguments->begin();

	// Iteration on I
	for(SetArgumentsIterator it = I.begin(); it != I.end(); it++, yt++){
		// New Argument, which we have to insert in gamma_reduced
		Argument *toAdd=new Argument(**it, gamma_reduced, index);
		//cout << "current argument: "<<(*it)->getName()<<endl<<"old af: "<<*(*it)->get_af() <<endl;
		++index;

		// attacks is the SetArguments which we use to iterate, we remove the suspended attacks from attacks1
		SetArguments * attacks = new SetArguments(*(*it)->get_attacks());
		SetArguments * attacks1 = new SetArguments(*(*it)->get_attacks());
		for(SetArgumentsIterator jt = attacks->begin(); jt != attacks->end(); jt++){
			if(!I.exists(*jt))
				attacks1->remove(*jt);
		}

		// set attacks1 as the set of attacks of the argument to add in the reducedAF
		toAdd->set_attacks(attacks1);

		// the same of attacks
		SetArguments * attackers = new SetArguments(*(*it)->get_attackers());
		SetArguments * attackers1 = new SetArguments(*(*it)->get_attackers());
		for(SetArgumentsIterator jt = attackers->begin(); jt != attackers->end(); jt++){
			if(!I.exists(*jt))
				attackers1->remove(*jt);
		}

		toAdd->set_attackers(attackers1);

		// add of the new argument in the reducedAF
		gamma_reduced->arguments->add_Argument(toAdd);

	}

	// iterate on the reduced_AF, necessary to adjust the index number of the new set of Argument 
	SetArguments * args_gamma = gamma_reduced->get_arguments();
	for(SetArgumentsIterator it = args_gamma->begin(); it != args_gamma->end(); it++){

		// take the set of attacks based on the old Argument and adjust it with the new Arguments which stay in gamma_reduced
		SetArguments * attacks = (*it)->get_attacks();
		SetArguments * new_attacks = attacks->adjust_set(args_gamma);
		(*it)->set_attacks(new_attacks);

		// the same of attacks
		SetArguments * attackers = (*it)->get_attackers();
		SetArguments * new_attackers = attackers->adjust_set(args_gamma);
		(*it)->set_attackers(new_attackers);
	}
	return *gamma_reduced;
}


/**
 * @brief 		Method for printing an AF
 */
ostream& operator<<(ostream& out, const AF& framework){
	SetArguments * argomenti=framework.get_arguments();
	out <<*argomenti<<endl;
	for(SetArgumentsIterator it=argomenti->begin();it!=argomenti->end();it++){
		out << (*it)->getName() << " attacks: " << *(*it)->get_attacks() << " attackers: "<< *(*it)->get_attackers()<<endl;
	}
	return out;
}
