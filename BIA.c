#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DIGITS 309 // 2^1024 binary represents 309 digits in decimal
#define DIGITS_PER_GROUP 9 // groups of 9 decimal digits are made
#define GROUP_MOD 1000000000

typedef enum { FAILURE, SUCCESS } ResultCode; // status code
typedef struct {
    int negativeFlag; // 1 if negative, else 0
    int* digitGroups; // 9-digit groups
    int groupCount; // no of grps
} BigNum;


int max(int a, int b);
ResultCode parseStringToNum(char* str, BigNum* num);
void printBigNum(BigNum* num);
BigNum addBigNumbers(BigNum* num1, BigNum* num2);
BigNum subtractBigNumbers(BigNum* num1, BigNum* num2);
BigNum multiplyBigNumbers(BigNum* num1, BigNum* num2);


int main() {
    char* str1 = (char*)malloc(sizeof(char) * (MAX_DIGITS + 2));
    char* str2 = (char*)malloc(sizeof(char) * (MAX_DIGITS + 2));
    char* temp = (char*)malloc(sizeof(char) * (MAX_DIGITS + 2));
    temp[0] = '+';
    temp[1] = '\0';
    ResultCode res1, res2;
    BigNum num1, num2;
    BigNum result;
    int operation;

    printf("Enter the first number with + or - in the start:\n");
    scanf("%s", str1);

    printf("Enter the second number with + or - in the start:\n");
    scanf("%s", str2);

    if (str1[0] != '+' && str1[0] != '-'){
        strcat(temp, str1);
        strcpy(str1, temp);
    }
    if (str2[0] != '+' && str2[0] != '-'){
        temp[1] = '\0';
        strcat(temp, str2);
        strcpy(str2, temp);
    }

    res1 = parseStringToNum(str1, &num1);
    res2 = parseStringToNum(str2, &num2);

    if (res1 == SUCCESS && res2 == SUCCESS) {
        printf("Enter the operation:\n");
        printf("1. Addition     2. Subtraction     3. Multiplication \n");
        scanf("%d", &operation);

        if (operation == 1) {
            result = addBigNumbers(&num1, &num2);
        } else if (operation == 2) {
            if (num1.negativeFlag == num2.negativeFlag){
                if (num1.groupCount > num2.groupCount)
                    result = subtractBigNumbers(&num1, &num2);
                else if (num2.groupCount > num1.groupCount){
                    result = subtractBigNumbers(&num2, &num1);
                    result.negativeFlag = 1-result.negativeFlag;
                }
                else if (num1.digitGroups[num1.groupCount-1] > num2.digitGroups[num2.groupCount-1])
                    result = subtractBigNumbers(&num1, &num2);
                else{
                    result = subtractBigNumbers(&num2, &num1);
                    result.negativeFlag = 1-result.negativeFlag;
                }
            }
            else{
                result = subtractBigNumbers(&num1, &num2);
            }
        } else if (operation == 3) {
            result = multiplyBigNumbers(&num1, &num2);
        } else {
            printf("Invalid operation. Please specify 1, 2, or 3.\n");
            return 1;
        }
        printBigNum(&result);
        free(num1.digitGroups);
        free(num2.digitGroups);
        free(result.digitGroups);
    } else {
        printf("Invalid input. Please enter valid numbers.\n");
    }

    free(str1);
    free(str2);
    free(temp);

    return 0;
}


int max(int a, int b) {
    return (a >= b) ? a : b;
} // to makecode more readable


ResultCode parseStringToNum(char* str, BigNum* num) {
    ResultCode result = SUCCESS;
    int strLen = strlen(str);
    int count = 0;
    int groups = (strLen / DIGITS_PER_GROUP) + (strLen % DIGITS_PER_GROUP != 0);
    int power = 1;
    int localNum = 0, endFlag = 0;

    if (str[0] == '+') {
        num->negativeFlag = 0;
    } else if (str[0] == '-') {
        num->negativeFlag = 1;
    } else {
        result = FAILURE;
    }

    if (result == SUCCESS) {
        num->groupCount = groups;
        int index = 0;
        num->digitGroups = (int*)malloc(sizeof(int) * (num->groupCount));

        for (int i = strLen - 1; i >= 1; i--) {
            int digit = (int)str[i] - 48;
            localNum = localNum + (digit * power);
            count++;
            power *= 10;
            if (i == 1) {
                endFlag = 1;
            }
            if (count == DIGITS_PER_GROUP || endFlag == 1) {
                num->digitGroups[index] = localNum;
                count = 0;
                localNum = 0;
                power = 1;
                index++;
            }
        }
    }

    return result;
}


void printBigNum(BigNum* num) {
    int groupCount = num->groupCount;
    printf((num->negativeFlag == 0) ? "+" : "-");
    int notZeroFlag = 0;
    for (int i = groupCount - 1; i >= 0; i--) {
        if (!notZeroFlag && num->digitGroups[i] != 0){
            notZeroFlag = 1;
            printf("%d", num->digitGroups[i]);
        }
        else if (notZeroFlag)
            printf("%09d", num->digitGroups[i]);
    }
    if (notZeroFlag == 0)
        printf("0");
    printf("\n");
}


BigNum addBigNumbers(BigNum* num1, BigNum* num2) {
    BigNum temp, result;
    long long carry = 0, sum;
    int groupCount1 = num1->groupCount, groupCount2 = num2->groupCount;

    if (num1->negativeFlag == num2->negativeFlag) {
        result.negativeFlag = num1->negativeFlag;
        result.groupCount = max(groupCount1, groupCount2) + 1;
        result.digitGroups = (int*)malloc(sizeof(int) * (result.groupCount));

        for (int i = 0; i < result.groupCount; i++) {
            sum = carry;
            if (i < groupCount1) sum += num1->digitGroups[i];
            if (i < groupCount2) sum += num2->digitGroups[i];

            result.digitGroups[i] = sum % GROUP_MOD;
            carry = sum / GROUP_MOD;
        }
    } else if (num1->negativeFlag == 1) {
        temp = *num1;
        temp.negativeFlag = 0;
        result = subtractBigNumbers(num2, &temp);
    } else if (num2->negativeFlag == 1) {
        temp = *num2;
        temp.negativeFlag = 0;
        result = subtractBigNumbers(num1, &temp);
    }

    return result;
}


BigNum subtractBigNumbers(BigNum* num1, BigNum* num2) {
    BigNum temp, result;
    long long diff, borrow = 0, carry = 0;
    int groupCount1 = num1->groupCount, groupCount2 = num2->groupCount;

    if (num1->negativeFlag == num2->negativeFlag) {
        result.negativeFlag = num1->negativeFlag;
        result.groupCount = max(groupCount1, groupCount2);
        result.digitGroups = (int*)malloc(sizeof(int) * (result.groupCount));

        for (int i = 0; i < result.groupCount; i++) {
            diff = num1->digitGroups[i] - borrow;
            if (i < groupCount2) 
                diff -= num2->digitGroups[i];
            if (diff < 0) {
                borrow = 1;
                diff += GROUP_MOD;
            } else {
                borrow = 0;
            }

            result.digitGroups[i] = diff;
        }
    } else {
        temp = *num2;
        temp.negativeFlag = !(temp.negativeFlag);
        result = addBigNumbers(num1, &temp);
    }

    return result;
}


BigNum multiplyBigNumbers(BigNum* num1, BigNum* num2) {
    int groupCount1 = num1->groupCount;
    int groupCount2 = num2->groupCount;
    unsigned long long i, j, sum, carry, product;
    BigNum result;

    result.groupCount = groupCount1 + groupCount2;
    result.digitGroups = (int*)malloc(sizeof(int) * result.groupCount);

    if (num1->negativeFlag == num2->negativeFlag) {
        result.negativeFlag = 0;
    } else {
        result.negativeFlag = 1;
    }

    for (int i = 0; i < result.groupCount; i++) {
        result.digitGroups[i] = 0;
    }

    for (i = 0; i < groupCount1; i++) {
        carry = 0;
        for (j = 0; j < groupCount2; j++) {
            unsigned long long num1Chunk = num1->digitGroups[i];
            unsigned long long num2Chunk = num2->digitGroups[j];
            sum = carry + (result.digitGroups[i + j]) + (num1Chunk * num2Chunk);
            int entry = sum % GROUP_MOD;
            result.digitGroups[i + j] = entry;
            carry = sum / GROUP_MOD;
        }
        if (carry != 0) {
            result.digitGroups[i + groupCount2] = carry;
        }
    }

    return result;
}
