/*
 * =====================================================================================
 *
 *       Filename:  rawcomm.hh
 *
 *    Description:  Raw protocol communication implementation
 *
 *        Version:  1.0
 *        Created:  11/09/2016 09:13:17 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Michael Peng
 *   Organization:  Adaline E. Kent Middle School
 *
 * =====================================================================================
 */

short proto_out(short prot)
{
	cout << "ProtoOut => " << prot << endl;
	return 0;
}

short proto_query(short query)
{
	cout << "Query " << query << ">";
	short input;
	cin >> input;
	return input;
}

short board_out(const Board& brd)
{
	cout << brd[0] << brd[1] << brd[2] << endl
		<< brd[3] << brd[4] << brd[5] << endl
		<< brd[6] << brd[7] << brd[8] << endl;
	return 0;
}
