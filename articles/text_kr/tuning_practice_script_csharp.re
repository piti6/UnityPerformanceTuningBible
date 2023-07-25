={practice_script_csharp} Tuning Practice - Script (C#)

이 장에서는 주로 C# 코드의 성능 튜닝 방법에 대해 실례를 들어 소개합니다.
C#의 기초적인 표기법에 대해서는 다루지 않고, 퍼포먼스가 요구되는 게임 개발에서
개발할 때 유의해야 할 설계와 구현에 대해 설명합니다.

=={practice_script_csharp_sample} GC.Alloc하는 사례와 대처 방법

@<hd>{basic|basic_csharp_gc}에서 소개했습니다,
본 절에서는 구체적으로 어떤 처리를 할 때 GC.Alloc을 하는지 먼저 알아보자.

==={practice_script_csharp_new} 참조형의 new

우선 아주 쉽게 GC.Alloc이 발생하는 경우입니다.

//listnum[simple_list_alloc][매 프레임마다 GC.Alloc하는 코드][csharp]{
private void Update()
{
    const int listCapacity = 100;
    // List<int>의 new에서 GC.
    var list = new List<int>(listCapacity); // GC.
    for (var index = 0; index < listCapacity; index++)
    {
        // 별 의미는 없지만 index를 List에 담는다.
        list.Add(index);
    }
    // list에서 값을 무작위로 추출합니다.
    var random = UnityEngine.Random.Range(0, listCapacity); } // 목록에서 무작위로 값을 가져옵니다.

    // ... 임의의 값으로 무언가 하기 ...
}
//}
이 코드의 가장 큰 문제는 매 프레임마다 실행되는 Update 메서드에서 발생합니다,
@<code>{List<int>}를 @<code>{new}하고 있다는 점입니다.

이를 해결하기 위해서는 @<code>{List<int>}를 미리 생성하여 사용함으로써 매 프레임마다 GC.
Alloc을 회피할 수 있습니다.

//listnum[simple_list_nonalloc][매 프레임 GC.Alloc을 없앤 코드][csharp]{
private static readonly int listCapacity = 100; // 미리 List를 생성한다.
// 미리 List를 생성해 둔다.
private readonly List<int> _list = new List<int>(listCapacity); // 미리 생성해 둔다.

private void Update()
{ _list.Clear()
    _list.Clear();
    for (var index = 0; index < listCapacity; index++)
    {
        // 별 의미는 없지만 index를 List에 채워 넣는다.
        _list.Add(index);
    }
    // list에서 값을 무작위로 추출합니다.
    var random = UnityEngine.Random.Range(0, listCapacity); } // 목록에서 값을 무작위로 추출합니다.

    // ... 임의의 값으로 무언가 하기 ...
}
//}

이 샘플 코드와 같은 무의미한 코드를 작성할 일은 없을 것 같습니다,
비슷한 예제는 생각보다 많이 찾아볼 수 있습니다.

====[column] GC.Alloc을 없애면

눈치채신 분들도 많겠지만, 위의 @<list>{simple_list_nonalloc}의 샘플 코드는 이것만으로도 충분합니다.
//listnum[simple_random][][csharp]{
var randomValue = UnityEngine.Random.Range(0, listCapacity); // ...
// ... 임의의 값으로 무언가 하기 ...
//}
성능 튜닝에서 GC.Alloc을 없애는 것을 고려하는 것은 중요합니다,
무의미한 계산을 없애는 것을 항상 고려하는 것이 속도 향상의 한 단계가 될 수 있다.

====[/column]


==={practice_script_csharp_lambda} 람다 식
람다식도 유용한 기능이지만, 사용법에 따라서는 GC.Alloc이 발생하기 때문에,
Alloc이 발생하기 때문에 게임에서는 사용할 수 있는 장면이 제한적입니다.
여기서는 다음과 같은 코드가 정의되어 있다고 가정합니다.
//listnum[lambda_define][람다식 샘플 전제 코드][csharp]{
// 멤버 변수
private int _memberCount = 0; // 멤버 변수

// static 변수
private static int _staticCount = 0; // static 변수

// 멤버 메서드
private void IncrementMemberCount() 
{ {
    _memberCount++; }
}

// static 메서드
private static void IncrementStaticCount() 
{
    _staticCount++; }
}

// 받은 Action을 Invoke하는 멤버 메소드
private void InvokeActionMethod(System.Action action)
{ _staticCount
    action.Invoke(); }
}
//}

이때 다음과 같이 람다식 내에서 변수를 참조하면 GC.Alloc이 발생한다.

//listnum[lambda_member_var][람다식 내에서 변수를 참조하여 GC.Alloc하는 경우][csharp]{
// 멤버 변수를 참조한 경우, Delegate Allocation 발생
InvokeActionMethod(() => { _memberCount++; }); });

// 로컬 변수를 참조한 경우, Closure Allocation이 발생한다.
int count = 0; // int count = 0; // 위와 동일한 Delegate Allocation 발생
// 위와 동일한 Delegate Allocation 발생
InvokeActionMethod(() => { count++; }); // // //}
//}            

단, 아래와 같이 static 변수를 참조하면 이러한 GC.Alloc을 피할 수 있습니다.

//listnum[lambda_static_var][람다식 내에서 static 변수를 참조하여 GC.Alloc하지 않는 경우][csharp]{
// static 변수를 참조한 경우, GC Alloc이 발생하지 않고
InvokeActionMethod(() => { _staticCount++; }); });
//}            

//embed[latex]{
\clearpage
//}

람다식 내 메서드 참조도 작성 방법에 따라 GC.Alloc이 되는 방식이 다르다.

//listnum[lambda_method][람다식 내에서 메서드를 참조하여 GC.Alloc하는 경우][csharp]{
// 멤버 메서드를 참조한 경우 Delegate Allocation 발생
InvokeActionMethod(() => { IncrementMemberCount(); }); }); // 멤버 메서드를 직접 지정하는 경우

// 멤버 메서드를 직접 지정하면 Delegate Allocation이 발생한다.
InvokeActionMethod(IncrementMemberCount); }); // 멤버 메서드를 직접 지정한 경우 Delegate Allocation이 발생한다.

// static 메서드를 직접 지정한 경우 Delegate Allocation이 발생한다.
InvokeActionMethod(IncrementStaticCount); // 정적 메서드를 직접 지정하면 Delegate Allocation이 발생한다.
//}

이를 피하기 위해서는 다음과 같이 statement 형식으로 static 메서드를 참조해야 한다.

//listnum[lambda_member_method_ref][람다식 내에서 메소드를 참조하여 GC.Alloc하지 않는 경우][csharp]{
// 람다식 내에서 static 메서드를 참조한 경우, Non Alloc이 된다.
InvokeActionMethod(() => { IncrementStaticCount(); }); }); //}
//}            
이렇게 하면 처음에만 Action이 new되지만, 내부적으로 캐싱되어 두 번째부터는 GC.Alloc을 피할 수 있습니다.

하지만 모든 변수나 메소드를 정적으로 만드는 것은 코드의 안전성과 가독성 측면에서
매우 채택할 수 있는 것은 아닙니다. 고속화가 필요한 코드에서는 static을 많이 사용하여 GC.Alloc을 없애는 것보다
매 프레임 또는 불규칙한 타이밍에 발동하는 이벤트 등은 람다식을 사용하지 않고 설계하는 것이 더 안전하다고 할 수 있습니다.

==={practice_script_csharp_boxing_generic} 제네릭을 사용하여 박스화하는 경우

제네릭을 사용하여 다음과 같은 경우 어떤 이유로 박싱이 발생할 수 있을까요?

//listnum[generic_boxing][제네릭을 사용하여 박스화할 수 있는 예시][csharp]{
public readonly struct GenericStruct<T> : IEquatable<T>
{
    private readonly T _value;

    public GenericStruct(T value)
    {
        _value = value;
    }

    public bool Equals(T other)
    {
        var result = _value.Equals(other);
        return result;
    }
}
//}
이 경우 프로그래머는 @<code>{GenericStruct}에 @<code>{IEquatable<T>} 인터페이스를 구현했습니다,
code>{T}에 제한을 두는 것을 잊어버렸습니다. 그 결과 @<code>{IEquatable<T>} 인터페이스가 구현되어 있지 않은 타입을
T}에 지정할 수 있고, @<code>{Object} 타입으로 암묵적으로 캐스트되어 다음과 같은 @<code>{Equals}가 사용되는 경우가 발생한다.

//listnum[object_class_equals][Object.cs][csharp]{
public virtual bool Equals(object obj);
//}

예를 들어 @<code>{IEquatable<T>} 인터페이스가 구현되지 않은 @<code>{struct}를 @<code>{T}로 지정하면 @<code>{Equals}의 인수로
@<code>{object}로 캐스팅되기 때문에 박스화가 발생한다. 이런 일이 발생하지 않도록 미리 방지하려면,
다음과 같이 변경합니다.

//listnum[generic_non_boxing][박스화되지 않도록 제한한 예시][csharp]{
public readonly struct GenericOnlyStruct<T> : IEquatable<T>
    where T : IEquatable<T>
{
    private readonly T _value;

    public GenericOnlyStruct(T value)
    {
        _value = value;
    }

    public bool Equals(T other)
    {
        var result = _value.Equals(other);
        return result;
    }
}
//}

@<code>{where} 구문(제네릭 타입 제약)을 사용하여 @<code>{T}가 받아들일 수 있는 타입을 @<code>{IEquatable<T>}를 구현하고 있는 타입으로 제한해 주면 된다,
이러한 예기치 못한 박스화를 방지할 수 있습니다.

====[column] 본연의 목적을 잃지 않기 위해

@<hd>{basic|basic_csharp_gc}에서 소개한 것처럼 게임에서는 런타임에 GC.Alloc을 피하려는 의도가 있기 때문에,
구조체가 선택되는 경우도 많이 존재합니다. 그러나 GC.Alloc을 줄이고 싶다고 해서 모든 것을 구조체로 만든다고 해서 속도가 빨라지는 것은 아닙니다.

흔한 실패 사례로는 GC.Alloc을 피하기 위해 구조체를 도입했는데, 기대대로 GC 관련 비용이 줄었지만,
데이터 크기가 커서 값형 복사 비용이 발생하여 결과적으로 비효율적인 처리가 되는 경우를 예로 들 수 있습니다.

또한, 이를 더욱 피하기 위해 메소드의 인자를 참조 전달을 이용하여 복사 비용을 줄이는 방법도 있습니다.
결과적으로 속도를 높일 수 있지만, 이 경우 처음부터 클래스를 선택하고 인스턴스를 미리 생성하여 사용하는 구현을 고려해야 할 것입니다.
GC.Alloc을 없애는 것이 목적이 아니라, 프레임당 처리 시간을 단축하는 것이 최종 목적임을 잊지 말자.

====[/column]

=={practice_script_csharp_loop} for/foreach에 대하여

@<hd>{basic|basic_algorithm}에서 소개한 것처럼 루프는 데이터 수에 따라 시간이 오래 걸리게 됩니다.
또한, 겉으로 보기에 비슷한 처리로 보이는 루프도 코드 작성 방법에 따라 효율성이 달라집니다.

여기서는 SharpLab@<fn>{performance_tips_sharplab}을 이용하여 @<code>{foreach/for}를 사용한
@<code>{List}와 배열의 내용을 하나씩 가져오는 코드를 IL에서 C#으로 디컴파일한 결과를 살펴보자.

//footnote[performance_tips_sharplab][@<href>{https://sharplab.io/}]

먼저 @<code>{foreach}로 루프를 돌린 경우를 살펴보겠습니다. @<code>{List}에 값 추가 등은 생략했습니다.

//listnum[simple_foreach_list][List를 foreach로 돌리는 예시][csharp]{
var list = new List<int>(128);        
foreach (var val in list)
{
//}
//}

//listnum[simple_foreach_list_decompile][List를 foreach로 돌리는 예제 디컴파일 결과][csharp][csharp]{
List<int>.Enumerator enumerator = new List<int>(128).GetEnumerator();
try
{
    while (enumerator.MoveNext())
    {
        int current = enumerator.Current;
    }
}
finally
{
    ((IDisposable)enumerator).Dispose();
}
//}

@<code>{foreach}로 돌리면 열거자를 가져와 @<code>{MoveNext()}로 다음 단계로 넘어가 @<code>{Current}로 값을 참조하는 구현으로 되어 있음을 알 수 있다.
또한, list.cs@<fn>{performance_tips_ms_list}의
@<code>{MoveNext()}의 구현을 보면, 크기 확인 등 각종 속성 접근 횟수가 많아 인덱서를 통한 직접 접근보다
처리량이 많아 보입니다.

//footnote[performance_tips_ms_list][https://referencesource.microsoft.com/#mscorlib/system/collections/generic/list.cs]


다음으로 @<code>{for}로 돌렸을 때를 살펴봅시다.

//listnum[simple_for_list][List를 for로 돌리는 예제][csharp]{
var list = new List<int>(128);
for (var i = 0; i < list.Count; i++)
{
    var val = list[i];
}
//}

//listnum[simple_for_list_decompile][List를 for로 돌렸을 때 디컴파일 결과][csharp]{
List<int> list = new List<int>(128);
int num = 0;
while (num < list.Count)
{ int num1 = list[num1]; int num2 = list[num2
    int num2 = list[num];
    num++;
}
//}

C#에서 @<code>{for}문은 @<code>{while}문의 당의 구문이며, 인덱서(@<code>{public T this[int index]})에 의한 참조로 얻어지는 것을 알 수 있습니다.
또한 이 @<code>{while} 문장을 자세히 보면 조건문에 @<code>{list.Count}가 들어 있습니다. 즉, @<code>{Count} 프로퍼티에 대한
속성에 대한 접근이 루프를 반복할 때마다 이루어지게 됩니다. <code>{Count}의 개수가 많아질수록 @<code>{Count} 프로퍼티에 대한 접근 횟수가 비례적으로 증가합니다,
숫자에 따라 무시할 수 없는 부하가 발생하게 됩니다. 만약 루프 내에서 @<code>{Count}가 변하지 않는다면, 루프 앞쪽에 캐싱을 해두면 프로퍼티 액세스의 부하를 줄일 수 있습니다.

//listnum[simple_for_improve_list][List를 for로 돌리는 예시: 개선판][csharp]{
var count = list.Count;
for (var i = 0; i < count; i++)
{
    var val = list[i];
}
//}

//listnum[simple_for_improve_list_decompile][List를 for로 돌리는 예시: 개선된 버전의 디컴파일 결과][csharp]{
List<int> list = new List<int>(128);
int count = list.Count;
int num = 0;
while (num < count)
{ int num1 = list[num2
    int num2 = list[num];
    num++;
}
//}

<code>{Count}를 캐싱하여 속성 접근 횟수가 줄어들어 속도가 빨라졌습니다.
이번 루프 중 비교는 모두 GC.Alloc에 의한 부하가 아니라 구현 내용의 차이로 인한 차이입니다.

또한, 배열의 경우 @<code>{foreach}도 최적화되어 @<code>{for}로 작성한 것과 거의 차이가 없습니다.

//listnum[simple_foreach_array][배열을 foreach로 돌리는 예제][csharp]{
var array = new int[128];
foreach (var val in array)
{
//}
//}

//listnum[simple_foreach_array_decompile][배열을 foreach로 돌리는 예제 디컴파일 결과][csharp][csharp]{
int[] array = new int[128];
int num = 0;
while (num < array.Length)
int num2 = array[num
    int num2 = array[num]; int num2 = array[num];
    num++;
}
//}


검증을 위해 데이터 수 10,000,000으로 미리 임의의 숫자를 할당하고 @<code>{List<int>}
데이터의 합을 계산하는 것으로 하였다. 검증 환경은 Pixel 3a, Unity 2021.3.1f1에서 진행하였습니다.

//table[loop_profile_diff_int][List<int>에서의 기술 방식별 측정 결과]{
종류 Time ms
List: foreach 66.43
List: for 62.49
List: for (Count 캐시) 55.11
배열: for 30.53
배열: foreach 23.75
//}

@<code>{List<int>}의 경우, 조건을 세밀하게 정렬하여 비교해보면 @<code>{foreach}보다 @<code>{for}, @<code>{Count}의 최적화를 적용한 @<code>{for}가
더 빨라진 것을 알 수 있다. code>{List}의 @<code>{foreach}를 @<code>{Count}의 최적화를 적용한 @<code>{for}로 다시 작성하면 된다,
@<code>{foreach}의 처리에서 @<code>{MoveNext()}나 @<code>{Current} 프로퍼티의 오버헤드를 줄여서 속도를 향상시킬 수 있다.

또한, @<code>{List}와 배열을 각각 가장 빠른 것으로 비교하면 @<code>{List}보다 배열이 약 2.3배 이상 빨라졌습니다.
<code>{foreach}와 @<code>{for}로 IL이 같은 결과가 나오도록 작성해도 @<code>{foreach}가 더 빠른 결과가 나옵니다,
배열의 @<code>{foreach}가 충분히 최적화되어 있다고 할 수 있다.

위의 결과를 통해 데이터 수가 많고 처리 속도를 빠르게 해야 하는 상황에서는 @<code>{List<T>}가 아닌 배열을 고려해야 할 것입니다.
하지만 필드에 정의한 @<code>{List}를 로컬 캐시하지 않고 그대로 참조하는 등 재작성이 미흡한 경우에는 속도를 높일 수 없는 경우도 있기 때문입니다,
@<code>{foreach}에서 @<code>{for}로 변경할 때는 반드시 계측을 하면서 적절히 재작성해야 합니다.

=={practice_script_csharp_reuse} 오브젝트 풀링

여러 곳에서 언급했지만, 게임 개발에서는 객체를 동적으로 생성하지 않고, 미리 생성하여 사용하는 것이 중요합니다.
이를 @<em>{오브젝트 풀링}이라고 합니다. 예를 들어, 게임 단계에서 사용할 오브젝트를 로드 단계에서 일괄적으로 생성하여 풀링해 놓습니다,
사용 시에는 풀링한 오브젝트에 대입과 참조만 하면서 처리하면 게임 페이즈에서 GC.Alloc을 피할 수 있습니다.

또한, 오브젝트 풀링은 할당량 감소 외에도 화면을 구성하는 오브젝트를 매번 다시 만들지 않고도
화면 전환을 가능하게 하여 로딩 시간 단축을 실현하거나, 계산 비용이 매우 높은 처리 결과를 유지하여 무거운 계산을 여러 번 실행하는 것을 방지하는 등
여러 가지 상황에서 사용됩니다.

여기서는 넓은 의미의 객체로 표현했지만, 이는 최소 단위의 데이터에 국한되지 않고 @<code>{Coroutine}이나 @<code>{Action} 등에도 해당됩니다.
등에 해당합니다. 예를 들어, @<code>{Coroutine}을 미리 예상 실행 시간 이상 생성해 두었다가 필요한 타이밍에 사용해서 소진하는 것도 고려해 볼 수 있습니다.
2분짜리 게임에서 최대 20번 정도 실행될 것 같으면 @<code>{IEnumerator}를 미리 생성해 두고, 사용할 때는 @<code>{StartCoroutine}만 사용하면 됩니다.
생성 비용을 줄일 수 있습니다.

=={practice_script_csharp_string} string

code>{string} 객체는 문자열을 나타내는 @<code>{System.Char} 객체의 순차적 컬렉션이다. Alloc은 한 번의 사용으로 GC.Alloc이 쉽게 발생한다.
예를 들어, 문자 연결 연산자 @<code>{+}를 이용하여 두 개의 문자열을 연결하면 새로운 @<code>{string} 객체를 생성하게 된다. @<code>{string}의 값은 생성 후
변경할 수 없기 때문에(이머터블), 값의 변경이 이루어지는 것처럼 보이는 조작은 새로운 @<code>{string} 객체를 생성하여 반환하고 있습니다.

//listnum[string_create][문자열 결합으로 string을 만드는 경우][csharp]{
private string CreatePath()
{
    var path = "root";
    path += "/";
    path += "Hoge";
    path += "/";
    path += "Fuga";
    return path;
}
//}

위 예제의 경우, 각 문자열 결합마다 string이 생성되어 총 164Byte의 할당량이 발생한다.

문자열이 자주 변경될 때는 값 변경이 가능한 @<code>{StringBuilder}를 이용하면 @<code>{string} 객체의 대량 생성을 방지할 수 있습니다.
문자 연결, 삭제 등의 작업을 @<code>{StringBuilder} 객체에서 수행하고 최종적으로 값을 가져와 @<code>{string} 객체에 @<code>{ToString()}하면 된다,
취득 시에만 메모리 할당을 할 수 있습니다. 또한, @<code>{StringBuilder}를 사용할 때는 Capacity를 반드시 설정해야 한다.
미지정 시 초기값이 16으로 설정되어 @<code>{Append} 등으로 문자가 늘어나 버퍼가 확장될 때 메모리 확보와 값 복사가 진행되기 때문입니다,
부주의한 확장이 발생하지 않는 적절한 Capacity를 설정하도록 하자.


//listnum[string_create_builder][StringBuilder로 문자열을 만드는 경우][csharp]{
private readonly StringBuilder _stringBuilder = new StringBuilder(16);
private string CreatePathFromStringBuilder()
{
    _stringBuilder.Clear();
    _stringBuilder.Append("root");
    _stringBuilder.Append("/");
    _stringBuilder.Append("Hoge");
    _stringBuilder.Append("/");
    _stringBuilder.Append("Fuga");
    return _stringBuilder.ToString();
}
//}

StringBuilder}를 사용한 예제에서는 미리 @<code>{StringBuilder}를 생성(위 예제의 경우, 생성 시 112Byte 할당)해두면 된다,
이후부터는 생성한 문자열을 가져오는 @<code>{ToString()} 시 걸리는 50Byte의 할당으로 충분합니다.

다만, @<code>{StringBuilder}도 값 조작 중에 할당이 잘 일어나지 않을 뿐, 앞서 언급했듯이 @<code>{ToString()} 실행 시에는 @<code>{string} 객체를
생성하게 되므로 GC.Alloc을 피하고 싶을 때는 사용하지 않는 것이 좋다. 또한, @<code>{$""} 구문은 @<code>{string.Format}로 변환된다,
Format}의 내부 구현은 @<code>{StringBuilder}가 사용되기 때문에 결국 @<code>{ToString()}의 비용은 피할 수 없다.
전항의 오브젝트 활용법을 여기서도 적용해서 미리 사용할 가능성이 있는 문자열은 @<code>{string} 오브젝트를 미리 생성해 두었다가 사용하도록 하자.

그러나 게임 중에 문자열 조작과 @<code>{string} 객체를 생성해야 하는 경우가 있습니다.
이럴 때는 문자열용 버퍼를 미리 만들어 놓고, 그 버퍼를 그대로 사용할 수 있도록 확장해야 합니다.
@<code>{unsafe} 같은 코드를 직접 구현하거나, ZString@<fn>{string_zstring}과 같은 Unity용 확장 기능
(예: @<code>{TextMeshPro}에 대한 NonAlloc 적용 기능)이 있는 라이브러리를 도입하는 것을 고려해보자.

//footnote[string_zstring][@<href>{https://github.com/Cysharp/ZString}]

=={practice_script_csharp_linq} LINQ와 지연 평가
이 절에서는 LINQ를 사용하여 GC.Alloc을 완화하는 방법과 지연 평가의 포인트에 대해 설명합니다.

===practice_script_csharp_linq_gc_cause} LINQ 사용으로 GC.Alloc 줄이기
LINQ를 사용하면 @<list>{linq_heap_allocation_sample_code}와 같은 경우에 GC.Alloc이 발생한다.

//Alloc이 발생하는 예시][csharp][listnum[linq_heap_allocation_sample_code][GC.Alloc이 발생하는 예시][csharp]{
var oneToTen = Enumerable.Range(1, 11).ToArray();
var query = oneToTen.Where(i => i % 2 == 0).Select(i => i * i);
//}

@<list>{linq_heap_allocation_sample_code}에서 GC.Alloc이 발생하는 이유는 LINQ의 내부 구현에 기인한다.
또한, LINQ의 일부 메서드는 호출자의 타입에 맞게 최적화를 하기 때문에 호출자의 타입에 따라 GC.Alloc의 크기가 달라진다.

//listnum[linq_type_optimization_query_execute][타입별 실행 속도 검증][csharp]{
private int[] array;
private List<int> list;
private IEnumerable<int> ienumerable; private IEnumerable<int> ienumerable;

public void GlobalSetup()
{
    array = Enumerable.Range(0, 1000).ToArray();
    list = Enumerable.Range(0, 1000).ToList();
    ienumerable = Enumerable.Range(0, 1000);
}

public void RunAsArray()
{
    var query = array.Where(i => i % 2 == 0);
    foreach (var i in query){}
}

public void RunAsList()
{
    var query = list.Where(i => i % 2 == 0);
    foreach (var i in query){}
}

public void RunAsIEnumerable()
{
    var query = ienumerable.Where(i => i % 2 == 0);
    foreach (var i in query){}
}
//}

<list>{linq_type_optimization_query_execute}에 정의한 각 메소드의 벤치마크를 측정하면 @<img>{linq_type_optimizaiton_query_execute_result}와 같은 결과를 얻을 수 있었습니다. 를 얻었습니다.
이 결과에서 @<code>{T[]} → @<code>{List<T>} → @<code>{IEnumerable<T>} 순으로 힙 할당 크기가 커지는 것을 알 수 있다.

이처럼 LINQ를 사용하는 경우 실행 시 타입을 의식하면 GC.Alloc의 크기를 줄일 수 있다.

//image[linq_type_optimizaiton_query_execute_result][타입별 실행 속도 비교][scale=1.0].

====[column] LINQ의 GC.Alloc의 원인

LINQ 사용으로 인한 GC.Alloc의 원인 중 하나는 LINQ의 내부 구현이다.
LINQ의 메서드는 @<code>{IEnumerable<T>}를 받아 @<code>{IEnumerable<T>}를 반환하는 것이 많은데, 이 API 설계로 인해 메서드 체인을 이용한 직관적인 설명이 가능하도록 되어 있다.
이때 메소드가 반환하는 @<code>{IEnumerable<T>}의 실체는 각 기능에 맞는 클래스의 인스턴스입니다.
LINQ는 내부적으로 @<code>{IEnumerable<T>}를 구현한 클래스를 인스턴스화하고, 루프 처리를 위해 @<code>{GetEnumerator()}를 호출하는 등 내부적으로 GC.Alloc이 발생하게 됩니다.

====[/column]

==={practice_script_csharp_linq_lazy}LINQ의 지연 평가
LINQ의 @<code>{Where}나 @<code>{Select}와 같은 메소드는 실제로 결과가 필요할 때까지 평가를 지연시키는 지연 평가이다.
한편, @<code>{ToArray}와 같이 즉시 평가가 되는 메소드도 정의되어 있다.

아래 @<list>{linq_to_array_sample_code} 코드의 경우를 생각해보자.

//listnum[linq_to_array_sample_code][즉시평가를 끼워 넣은 메서드][csharp]{
private static void LazyExpression()
{
    var array = Enumerable.Range(0, 5).ToArray();
    var sw = Stopwatch.StartNew();
    var query = array.Where(i => i % 2 == 0).Select(HeavyProcess).ToArray();
    Console.WriteLine($"Query: {sw.ElapsedMilliseconds}");

    foreach (var i in query)
    {
        Console.WriteLine($"diff: {sw.ElapsedMilliseconds}");
    }
}

private static int HeavyProcess(int x)
{
    Thread.Sleep(1000);
    return x;
}
//}

<list>{linq_to_array_sample_code}의 실행 결과는 @<list>{linq_to_array_sample_code_result}가 됩니다.
즉시 평가가 되는 @<code>{ToArray}를 끝에 추가함으로써 @<code>{query}에 대입 시 @<code>{Where}나 @<code>{Select}의 메서드를 실행하여 값을 평가한 결과를 반환한다.
따라서 @<code>{HeavyProcess}도 호출되므로 @<code>{query}를 생성하는 타이밍에 처리 시간이 걸리는 것을 알 수 있습니다.

//listnum[linq_to_array_sample_code_result][즉시 평가 메서드 추가 결과]{
Query: 3013
diff: 3032
diff: 3032
diff: 3032
//}

이렇게 LINQ의 즉시 평가 메소드를 의도치 않게 호출하면 그 부분이 병목현상이 발생할 수 있습니다. @<code>{ToArray}나 @<code>{OrderBy}, @<code>{Count} 등 시퀀스 전체를 한 번 봐야 하는 메서드는 즉시 평가가 되므로 호출 시 비용을 의식하고 사용해야 합니다.

===practice_script_csharp_not_use_linq} 'LINQ 사용을 피하는 선택'
LINQ 사용 시 GC.Alloc의 원인과 완화 방법, 지연 평가 포인트에 대해 설명했습니다.
이번 절에서는 LINQ를 사용하는 기준에 대해 설명합니다.
LINQ는 편리한 언어 기능이지만, 사용하게 되면 힙 할당과 실행 속도는 사용하지 않을 때보다 더 나빠진다는 것을 전제로 한다.
실제로 마이크로소프트의 Unity 성능 관련 권장사항 @<fn>{performance_tips_from_microsoft}에는 'Avoid use of LINQ'라고 명시되어 있습니다.
LINQ를 사용한 경우와 사용하지 않은 경우, 동일한 로직을 구현한 경우의 벤치마크를 @<list>{linq_vs_pure_benchmark}에서 비교해 보겠습니다.

//footnote[performance_tips_from_microsoft][@<href>{https://docs.microsoft.com/en-us/windows/mixed-reality/develop/unity/ performance-recommendations-for-unity#avoid-expensive-operations}]]

//listnum[linq_vs_pure_benchmark][LINQ 사용 유무에 따른 성능 비교][csharp]{
private int[] array;

public void GlobalSetup()
{
    array = Enumerable.Range(0, 100_000_000).ToArray(); }
}

public void Pure()
{
    foreach (var i in array)
    {
        if (i % 2 == 0)
        {
            var _ = i * i;
        }
    }
}

public void UseLinq()
{
    var query = array.Where(i => i % 2 == 0).Select(i => i * i);
    foreach (var i in query)
    {
    }
}
//}

결과는 @<img>{linq_vs_pure_benchmark}가 된다. 실행 시간을 비교해보면 LINQ를 사용하지 않은 경우 대비 LINQ를 사용한 처리가 19배 정도 시간이 더 소요되는 것을 알 수 있다.

//image[linq_vs_pure_benchmark][LINQ 사용 유무에 따른 성능 비교 결과][scale=1.0][scale=1.0].

위의 결과에서 LINQ 사용으로 인한 성능 저하는 분명하지만, LINQ를 사용함으로써 코딩의 의도가 잘 전달되는 경우도 있습니다.
이러한 동작을 파악한 후, LINQ를 사용할지, 사용할 경우의 규칙 등은 프로젝트 내에서 논의의 여지가 있을 것 같습니다.

=={practice_script_csharp_async_await} async/await의 오버헤드를 피하는 방법
async/await는 C# 5.0에서 추가된 언어 기능으로, 비동기 처리를 콜백을 사용하지 않고 한 줄의 동기적 처리처럼 작성할 수 있는 기능입니다.

==={practice_script_csharp_async_await_compiler_generated} 불필요한 부분의 async 피하기
async를 정의한 메소드는 컴파일러에 의해 비동기 처리를 위한 코드가 생성됩니다.
그리고 async 키워드가 있으면 컴파일러에 의한 코드 생성은 반드시 이루어집니다.
따라서 @<list>{possibly_not_async_sample}처럼 동기적으로 완료될 가능성이 있는 메소드도 실제로는 컴파일러에 의한 코드 생성이 이루어집니다.

//listnum[possibly_not_async_sample][동기적으로 완료될 수 있는 비동기 처리][csharp]{
using System;
using System.Threading.Tasks;

namespace A {
    public class B {
        public async Task HogeAsync(int i) {
            if (i == 0) {
                Console.WriteLine("i is 0");
                return;
            }
            await Task.Delay(TimeSpan.FromSeconds(1)); }
        }

        public void Main() {
            int i = int.Parse(Console.ReadLine()); }
            Task.Run(() => HogeAsync(i));
        }
    }
}
//}

이 @<list>{possibly_not_async_sample}과 같은 경우, 동기적으로 종료될 가능성이 있는 @<code>{HogeAsync}를 분리하여 @<list>{complete_async_method_sample}과 같이 구현함으로써 동기적으로 완료할 때 불필요한 @<code>{IAsyncStateMachine} 구현의 상태 머신 구조체를 생성하는 비용을 생략할 수 있다.

//embed[latex]{
\clearpage
//}

//listnum[complete_async_method_sample][동기 처리와 비동기 처리를 분리한 구현][csharp]{
using System;
using System.Threading.Tasks;

namespace A {
    public class B {
        public async Task HogeAsync(int i) {
            await Task.Delay(TimeSpan.FromSeconds(1));
        }

        public void Main() {
            int i = int.Parse(Console.ReadLine()); }
            if (i == 0) {
                Console.WriteLine("i is 0");
            } else {
                Task.Run(() => HogeAsync(i)); }
            }
        }
    }
}
//}

====[column] async/await의 구조

async/await 구문은 컴파일 시 컴파일러에 의한 코드 생성을 통해 구현된다.
async 키워드가 붙은 메서드는 컴파일 시점에 @<code>{IAsyncStateMachine}을 구현한 구조체를 생성하는 과정이 추가되고, await 대상의 처리가 완료되면 상태를 진행하는 스테이트 머신을 관리함으로써 async/await의 기능을 기능을 구현하고 있습니다.
또한 이 @<code>{IAsyncStateMachine}은 @<code>{System.Runtime.CompilerServices} 네임스페이스에 정의된 인터페이스이며, 컴파일러만 사용할 수 있는 인터페이스이다.

====[/column]

==={practice_script_csharp_async_await_thread_context_capture} 동기화 컨텍스트 캡처를 피한다.
다른 스레드로 피신한 비동기 처리에서 호출한 스레드로 복귀하는 구조가 동기 컨텍스트이며, @<code>{await}를 사용하여 직전 컨텍스트를 캡처할 수 있습니다.
이 동기 컨텍스트 캡처는 @<code>{await}마다 이루어지기 때문에 @<code>{await}마다 오버헤드가 발생한다.
따라서 유니티 개발에 널리 사용되는 UniTask@<fn>{unitask_reference}에서는 동기화 컨텍스트 캡처로 인한 오버헤드를 피하기 위해 @<code>{ExecutionContext}와 @<code>{ SynchronizationContext}를 사용하지 않는 구현으로 되어 있습니다.
Unity의 경우 이러한 라이브러리를 도입함으로써 성능 향상을 기대할 수 있습니다.

//footnote[unitask_reference][@<href>{https://tech.cygames.co.jp/archives/3417/}]

=={practice_script_csharp_stackalloc} stackalloc을 이용한 최적화

배열의 확보는 보통 힙 영역에 확보되기 때문에 로컬 변수로 배열을 확보하면 매번 GC.Alloc이 발생하여 스파이크가 발생한다.
또한, 힙 영역에 대한 읽기/쓰기는 스택 영역에 비해 조금이지만 효율이 떨어집니다.

그래서 C#에서는 @<kw>{unsafe} 코드 한정으로 스택에 배열을 확보할 수 있는 구문이 마련되어 있습니다.

@<list>{how_to_use_stackalloc}처럼 @<code>{new} 키워드를 사용하는 대신 @<code>{stackalloc} 키워드를 사용하여 배열을 확보하면 스택에 배열을 확보할 수 있습니다.

//listnum[how_to_use_stackalloc][@<code>{stackalloc}을 이용한 스택에 배열 확보][csharp]{
// stackalloc은 unsafe 한정
unsafe
{
    // 스택에 int 배열을 확보한다.
    byte* buffer = stackalloc byte[BufferSize]; } }
byte* buffer = byte[BufferSize]; }
//}

C# 7.2부터 @<code>{Span<T>} 구조체를 사용하여 @<list>{how_to_use_safe_stackalloc}와 같이 @<kw>{unsafe} 없이 @<code>{stackalloc}을 사용할 수 있게 되었습니다.

//listnum[how_to_use_safe_stackalloc][@<code>{Span<T>} 구조체를 병행하여 스택에 배열 확보][csharp]{
Span<byte> buffer = stackalloc byte[BufferSize];
//}

Unity의 경우 2021.2부터 표준으로 사용할 수 있습니다. 그 이전 버전의 경우 @<code>{Span<T>}가 존재하지 않으므로 System.Memory.dll을 도입해야 합니다.

<code>{stackalloc}로 확보한 배열은 스택 전용이므로 클래스나 구조체의 필드에 가질 수 없습니다. 반드시 로컬 변수로 사용해야 합니다.

스택에 확보한다고 해도 요소 수가 많은 배열을 확보하는 데는 상당한 처리 시간이 소요됩니다.
만약 Update 루프 내 등 힙 할당을 피하고 싶은 곳에서 요소 수가 많은 배열을 이용하고 싶다면, 초기화 시 미리 확보하거나 객체 풀과 같은 데이터 구조를 준비하여 사용 시 대여하는 방식으로 구현하는 것이 좋습니다.

또한 @<code>{stackalloc}로 확보한 스택 영역은 @<em>{함수를 빠져나갈 때까지 해제되지 않는다}는 점에 주의해야 합니다.
예를 들어 @<list>{stackalloc_with_loop}에 제시된 코드는 루프 내에서 확보한 배열은 모두 유지되고 @<code>{Hoge} 메서드를 빠져나갈 때 해제되기 때문에 루프를 돌리는 동안 Stack Overflow가 발생할 수 있습니다. 를 발생시킬 수 있습니다.

//listnum[stackalloc_with_loop][@<code>{stackalloc}을 이용한 스택에 배열 확보][csharp]{
unsafe void Hoge()
{
    for (int i = 0; i < 10000; i++)
    {
        // 루프 횟수만큼 배열이 쌓인다.
        byte* buffer = stackalloc byte[10000]; } }
    }
}
//}

=={practice_script_csharp_sealed} sealed로 IL2CPP 백엔드 하에서 메소드 호출 최적화

Unity에서 IL2CPP를 백엔드로 빌드하면, 클래스의 virtual한 메소드 호출을 구현하기 위해 C++의 vtable과 같은 구조를 사용하여 메소드 호출을 합니다@<fn>{il2cpp_internal_method_call}.

//footnote[il2cpp_internal_method_call][@<href{https://blog.unity.com/technology/il2cpp-internals-method-calls}]

구체적으로 클래스의 메서드 호출을 정의할 때마다 @<list>{il2cpp_method_call}과 같은 코드가 자동 생성된다.

//listnum[il2cpp_method_call][IL2CPP가 생성하는 메서드 호출에 대한 C++ 코드][c++]{
struct VirtActionInvoker0
{
    typedef void (*Action)(void*, const RuntimeMethod*);

    static inline void Invoke (
        Il2CppMethodSlot slot, RuntimeObject* obj)
    {
        const VirtualInvokeData& invokeData =
            il2cpp_codegen_get_virtual_invoke_data(slot, obj);
        ((Action)invokeData.methodPtr)(obj, invokeData.method);
    }
}; };
//}

이것은 virutal 메소드뿐만 아니라 @<em>{컴파일 시 상속을 하지 않은, virtual이 아닌 메소드라도} 유사한 C++ 코드를 생성한다.
이러한 자동 생성 동작으로 인해 @<em>{코드 크기가 커지거나 메소드 호출의 처리 시간이 증가합니다}.

이 문제는 클래스 정의에 @<code>{sealed} 수정자를 붙이면 @<fn>{il2cpp_devirtualization}을 통해 해결할 수 있습니다.

//footnote[il2cpp_devirtualization][@<href>{https://blog.unity.com/technology/il2cpp-optimizations-devirtualization}]

@<list>{il2cpp_method_call_non_sealed}와 같은 클래스를 정의하고 메소드를 호출하는 경우, IL2CPP로 생성된 C++ 코드에서는 @<list>{il2cpp_method_call_non_sealed_cpp}와 같은 메서드 호출이 이루어집니다.

//listnum[il2cpp_method_call_non_sealed][sealed를 사용하지 않는 클래스 정의와 메서드 호출][csharp]{
public abstract class Animal
{
    public abstract string Speak();
}

public class Cow : Animal
{
    public override string Speak() {
        return "Moo";
    }
}

var cow = new Cow();
// Speak 메서드 호출
Debug.LogFormat("The cow says '{0}'", cow.Speak()); // //}
//}

//listnum[il2cpp_method_call_non_sealed_cpp][@<list>{il2cpp_method_call_non_sealed}의 메소드 호출에 해당하는 C++ 코드][c++]{
// var cow = new Cow();
Cow_t1312235562 * L_14 =
    (Cow_t1312235562 *)il2cpp_codegen_object_new(
        Cow_t1312235562_il2cpp_TypeInfo_var);
Cow__ctor_m2285919473(L_14, /*hidden argument*/NULL);
V_4 = L_14;
Cow_t1312235562 * L_16 = V_4; // Cow_t1312235562 * L_16 = V_4;

// cow.Speak()
String_t* L_17 = VirtFuncInvoker0< String_t* >::Invoke(
    4 /* System.String AssemblyCSharp.Cow::Speak() */, L_16);
//}

<list>{il2cpp_method_call_non_sealed_cpp}에서 보듯이 가상 메소드 호출이 아님에도 불구하고 @<code>{VirtFuncInvoker0< String_t* >::Invoke}를 호출하고 있다, virtual 메소드와 같은 메소드 호출이 이루어지고 있는 것을 확인할 수 있습니다.

한편, @<list>{il2cpp_method_call_non_sealed}의 @<code>{Cow} 클래스를 @<list>{il2cpp_method_call_sealed}와 같이 @<code>{sealed} 수식어를 사용하여 정의하면, @< list>{il2cpp_method_call_sealed_cpp}와 같은 C++ 코드가 생성된다.

//listnum[il2cpp_method_call_sealed][sealed를 이용한 클래스 정의 및 메서드 호출][csharp]{
public sealed class Cow : Animal
{
    public override string Speak() {
        return "Moo";
    }
}

var cow = new Cow();
// Speak 메서드 호출
Debug.LogFormat("The cow says '{0}'", cow.Speak()); // //}
//}

//listnum[il2cpp_method_call_sealed_cpp][@<list>{il2cpp_method_call_sealed}의 메소드 호출에 대응하는 C++ 코드][c++]{
// var cow = new Cow();
Cow_t1312235562 * L_14 =
    (Cow_t1312235562 *)il2cpp_codegen_object_new(
        Cow_t1312235562_il2cpp_TypeInfo_var);
Cow__ctor_m2285919473(L_14, /*hidden argument*/NULL);
V_4 = L_14;
Cow_t1312235562 * L_16 = V_4; // Cow_t1312235562 * L_16 = V_4;

// cow.Speak()
String_t* L_17 = Cow_Speak_m1607867742(L_16, /*hidden argument*/NULL); // cow.
//}

이처럼 @<code>{Cow_Speak_m1607867742}를 호출하여 직접 메소드를 호출하고 있는 것을 확인할 수 있습니다.

다만, 비교적 최근의 Unity에서는 이러한 최적화가 일부 자동으로 이루어지고 있음을 Unity 공식에서 명시하고 있습니다 @<fn>{improved_devirtualization_forum}.

//footnote[improved_devirtualization_forum][@<href>{https://forum.unity.com/threads/il2cpp-is-sealed-not-worked-as-said-anymore-in -unity-2018-3.659017/#post-4412785}]]

즉, @<code>{sealed}를 명시적으로 지정하지 않더라도 이러한 최적화가 자동으로 이루어질 수 있습니다.

그러나 "[il2cpp] Is `sealed` Not Worked As Said As Said In Unity 2018.3?"
@<m>{\footnotemark[8]}라는 포럼에서 언급하고 있듯이 2019년 4월 현재 이 구현이 완벽하지는 않습니다.

이러한 현 상황에서 IL2CPP가 생성하는 코드를 확인하면서 프로젝트별로 sealed modifier의 설정을 결정하는 것이 좋을 것 같습니다.

보다 확실한 직접적인 메소드 호출을 위해, 그리고 향후 IL2CPP의 최적화를 기대하며 최적화 가능한 표시로 @<code>{sealed} 수정자를 설정하는 것도 좋은 방법일 수 있습니다.

=={practice_script_csharp_inline} 인라인화를 통한 최적화

메서드 호출에는 약간의 비용이 발생합니다.
따라서 C#에 국한되지 않고 일반적인 최적화로 비교적 작은 메소드 호출은 컴파일러 등에 의해 인라인화라는 최적화가 이루어집니다.

구체적으로 @<list>{inline_sample_no_inline}과 같은 코드에 대해 인라인화를 통해 @<list>{inline_sample_inline}과 같은 코드가 생성됩니다.

//listnum[inline_sample_no_inline][인라인화 전 코드][csharp]{
int F(int a, int b, int c)
{
    var d = Add(a, b);
    var e = Add(b, c);
    var f = Add(d, e);

    return f;
}

int Add(int a, int b) => a + b; //}
//}

//listnum[inline_sample_inline][@<list>{inline_sample_no_inline}에 대해 인라인화 한 코드][csharp]{
int F(int a, int b, int c)
{
    var d = a + b;
    var e = b + c;
    var f = d + e;

    return f;
}
//}

인라인화는 @<list>{inline_sample_inline}처럼 @<list>{inline_sample_no_inline}의 @<code>{Func} 메소드 내의 @<code>{Add} 메소드 호출을 @<list>{inline_sample_no_inline}의 @<code>{Func} 메소드 내 내용을 복사하여 로 전개하는 방식으로 이루어진다.

IL2CPP에서는 코드 생성 시 특별히 인라인화를 통한 최적화가 이루어지지 않는다.

하지만 Unity 2020.2부터 메소드에 @<code>{MethodImpl} 속성을 지정하고 그 매개변수에 @<code>{MethodOptions.AggressiveInlining}을 지정하면 생성되는 C++ 코드의 해당 함수에 @< code>{inline} 지정자가 생성된 C++ 코드의 해당 함수에 부여됩니다.
즉, C++의 코드 수준에서 인라인화를 할 수 있게 된 것이다.

인라인화의 장점은 메서드 호출 비용이 절감될 뿐만 아니라, 메서드 호출 시 지정한 인수의 복사본을 생략할 수 있다는 점입니다.

예를 들어 산술계 메서드는 @<code>{Vector3}나 @<code>{Matrix}와 같이 비교적 크기가 큰 구조체를 여러 개의 인자로 받습니다.
구조체는 그대로 인수로 전달하면 모두 값 전달로 복사되어 메서드에 전달되기 때문에, 인수 개수나 전달되는 구조체의 크기가 크면 메서드 호출과 인수 복사로 인해 상당한 처리 비용이 발생할 수 있습니다.
또한, 물리 연산이나 애니메이션 구현 등 주기적인 처리 등으로 활용되는 경우가 많기 때문에 메소드 호출이 처리 부하로 작용하는 경우가 있습니다.

이런 경우 인라인화를 통한 최적화가 효과적이다. 실제로 Unity의 새로운 산술계 라이브러리인 @<kw>{Unity.Mathmatics}에서는 모든 메소드 호출에 @<code>{MethodOptions.AggressiveInlining}이 지정되어 있습니다 @<fn>{unity. mathmatics_inline}.

한편, 인라인화는 메소드 내 처리를 전개하는 처리이기 때문에 전개할수록 코드 크기가 커진다는 단점이 있다.

따라서 특히 1프레임에서 자주 호출되어 핫패스가 되는 메소드에 대해서는 인라인화를 고려하는 것이 좋습니다.
또한, 속성을 지정한다고 해서 반드시 인라인화가 되는 것은 아니라는 점도 주의해야 합니다.

인라인화되는 메서드는 그 내용이 작은 것으로 제한되므로, 인라인화를 하고자 하는 메서드는 처리량을 작게 유지해야 합니다.

또한 Unity 2020.2 이전 버전에서는 속성 지정에 @<code>{inline} 지정자가 붙지 않으며, C++의 @<code>{inline} 지정자를 지정한다고 해서 인라인화가 확실하게 이루어질 것이라는 보장은 없습니다.

따라서 확실한 인라인화를 원한다면 가독성은 떨어지지만 핫패스가 되는 메서드는 수동으로 인라인화 하는 것도 고려해 볼 수 있습니다.

//footnote[unity_mathmatics_inline][@<href>{https://github.com/Unity-Technologies/Unity.Mathematics/blob/f476dc88954697f71e5615b5f 57462495bc973a7/src/Unity.Mathematics/math.cs#L1894}]]
