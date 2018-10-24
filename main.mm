#import <Foundation/Foundation.h>
#import "myobj.hpp"

//#import <stdio.h>

#define SMLND_DBG_LOG_M(M,X) printf(M, X)


@interface SampleClass:NSObject
- (void)sampleMethod:(int)val;
- (NSNumber *)multiplyA:(NSNumber *)a withB:(NSNumber *)b;
+(void)types;
@end

@implementation SampleClass

- (void)sampleMethod:(int)val {
   NSLog(@"Hello, World! \n");
   printf("Val = %d\n", val);
}

+(void)types {
    float *fp, tf = 10.5f;
    fp = &tf;

	NSLog(@"Storage size for char : %d \n", sizeof(char));
	NSLog(@"Storage size for short : %d \n", sizeof(short));
	NSLog(@"Storage size for wchar : %d \n", sizeof(wchar_t));
	NSLog(@"Storage size for int : %d \n", sizeof(int));
	NSLog(@"Storage size for long : %d \n", sizeof(long));
	NSLog(@"Storage size for float : %d \n", sizeof(float));
    NSLog(@"Storage size for double : %d \n", sizeof(double));
    NSLog(@"Storage size for pointer : %d \n", sizeof(*fp));
    SMLND_DBG_LOG_M("Storage size for double: %i \n", sizeof(double));
}

- (NSNumber *)multiplyA:(NSNumber *)a withB:(NSNumber *)b {
   float number1 = [a floatValue];
   float number2 = [b floatValue];
   float product = number1 * number2;
   NSNumber *result = [NSNumber numberWithFloat:product];
   return result;
}

@end

int main() {
   NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

   SampleClass *sampleClass = [[SampleClass alloc]init];
   NSNumber *a = [NSNumber numberWithFloat:10.5];
   NSNumber *b = [NSNumber numberWithFloat:10.0];   
   NSNumber *result = [sampleClass multiplyA:a withB:b];
   NSString *resultString = [result stringValue];
   NSLog(@"The product is %@",resultString);

   [SampleClass types];

   [pool drain];

    MyObj *mo = new MyObj(10);
    SMLND_DBG_LOG_M("%s", mo->name);
    delete mo;

   return 0;
}