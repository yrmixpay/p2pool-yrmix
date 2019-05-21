#include <Python.h>

static const long long COIN = 100000000;

double ConvertBitsToDouble(unsigned int nBits)
{
    int nShift = (nBits >> 24) & 0xff;

    double dDiff =
        (double)0x0000ffff / (double)(nBits & 0x00ffffff);

    while (nShift < 29)
    {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 29)
    {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;
}

long long static GetBlockBaseValue(int nBits, int nHeight, bool fTestNet = false)
{
    double dDiff = (double)0x0000ffff / (double)(nBits & 0x00ffffff);

    /* fixed bug caused diff to not be correctly calculated */
    if(nHeight > 4500 || fTestNet) dDiff = ConvertBitsToDouble(nBits);

    long long nSubsidy = 0;
    if(nHeight >= 5465) {
        if((nHeight >= 17000 && dDiff > 75) || nHeight >= 24000) { // GPU/ASIC difficulty calc
            // 2222222/(((x+2600)/9)^2)
            nSubsidy = (2222222.0 / (pow((dDiff+2600.0)/9.0,2.0)));
            if (nSubsidy > 25) nSubsidy = 25;
            if (nSubsidy < 5) nSubsidy = 5;
        } else { // CPU mining calc
            nSubsidy = (11111.0 / (pow((dDiff+51.0)/6.0,2.0)));
            if (nSubsidy > 500) nSubsidy = 500;
            if (nSubsidy < 25) nSubsidy = 25;
        }
    } else {
        nSubsidy = (1111.0 / (pow((dDiff+1.0),2.0)));
        if (nSubsidy > 500) nSubsidy = 500;
        if (nSubsidy < 1) nSubsidy = 1;
    }

    // LogPrintf("height %u diff %4.2f reward %i \n", nHeight, dDiff, nSubsidy);
    nSubsidy *= COIN;

    if(fTestNet){
        for(int i = 46200; i <= nHeight; i += 210240) nSubsidy -= nSubsidy/14;
    } else {
        // yearly decline of production by 7.1% per year, projected 21.3M coins max by year 2050.
        for(int i = 210240; i <= nHeight; i += 210240) nSubsidy -= nSubsidy/14;
    }

	/* Hard fork will activate on block 328008, reducing the block reward by 10 extra percent (allowing budget super-blocks) */
    if(fTestNet){
        if (nHeight > 77900+576) nSubsidy -= nSubsidy/10;
    } else {
        if (nHeight > 309759+(553*33)) nSubsidy -= nSubsidy/10; // 328008 - 10.0% - September 6, 2015
    }

    return nSubsidy;
}

static PyObject *yrmix_subsidy_getblockbasevalue(PyObject *self, PyObject *args)
{
    int input_bits;
    int input_height;
    if (!PyArg_ParseTuple(args, "ii", &input_bits, &input_height))
        return NULL;
    long long output = GetBlockBaseValue(input_bits, input_height);
    return Py_BuildValue("L", output);
}

static PyObject *yrmix_subsidy_getblockbasevalue_testnet(PyObject *self, PyObject *args)
{
    int input_bits;
    int input_height;
    if (!PyArg_ParseTuple(args, "ii", &input_bits, &input_height))
        return NULL;
    long long output = GetBlockBaseValue(input_bits, input_height, true);
    return Py_BuildValue("L", output);
}

static PyMethodDef yrmix_subsidy_methods[] = {
    { "GetBlockBaseValue", yrmix_subsidy_getblockbasevalue, METH_VARARGS, "Returns the block value" },
    { "GetBlockBaseValue_testnet", yrmix_subsidy_getblockbasevalue_testnet, METH_VARARGS, "Returns the block value for testnet" },
    { NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC initsib_subsidy(void) {
    (void) Py_InitModule("yrmix_subsidy", yrmix_subsidy_methods);
}
