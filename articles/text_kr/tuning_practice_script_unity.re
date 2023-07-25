={practice_script_unity} Tuning Practice - Script (Unity)

Unity에서 제공하는 기능을 무심코 사용하다 보면 예상치 못한 함정에 빠질 수 있습니다.
이 장에서는 Unity의 내부 구현과 관련된 성능 튜닝 방법을 실례와 함께 소개합니다.

=={practice_script_unity_event} 빈 Unity 이벤트 함수
@<code>{Awake}, @<code>{Start}, @<code>{Update} 등 Unity에서 제공하는 이벤트 함수가 정의되어 있는 경우,
실행 시 Unity 내부 리스트에 캐시되어 리스트의 반복에 의해 실행된다.

함수 내에서 아무런 처리를 하지 않더라도 정의되어 있는 것만으로도 캐시 대상이 되기 때문이다,
불필요한 이벤트 함수를 남겨두면 리스트가 커져 반복처리 비용이 증가하게 됩니다.

예를 들어 아래 샘플 코드와 같이 Unity에서 새로 생성한 스크립트에는 @<code>{Start}, @<code>{Update}가 처음부터 정의되어 있습니다,
이러한 함수가 불필요하다면 반드시 삭제해야 합니다.

//listnum[new_script][Unity에서 새로 생성한 스크립트][csharp]{
public class NewBehaviourScript : MonoBehaviour
{
    // 첫 번째 프레임 업데이트 전에 Start가 호출됩니다.
    void Start()
    {

    }

    // Update는 한 프레임에 한 번씩 호출됩니다.
    void Update()
    { } // Update는 프레임마다 한 번씩 호출된다.

    }
}
//}

=={practice_script_unity_tag_name} tag 및 name 접근
UnityEngine.Object}를 상속받은 클래스에는 @<code>{code>{tag} 속성과 @<code>{name} 속성이 제공됩니다.
객체의 식별에 유용한 속성이지만, 실제로는 GC.Alloc이 발생하고 있습니다.

각각의 구현을 UnityCsReference에서 인용했습니다.
둘 다 네이티브 코드로 구현된 처리를 호출하고 있음을 알 수 있습니다.

Unity에서는 스크립트를 C#으로 구현하지만, Unity 자체는 C++로 구현되어 있습니다.
C# 메모리 공간과 C++ 메모리 공간을 공유할 수 없기 때문에 C++ 측에서 C# 측으로 문자열 정보를 전달하기 위해 메모리 확보가 이루어집니다.
이는 호출할 때마다 이루어지므로 여러 번 접근하는 경우 캐싱을 해 두어야 합니다.

Unity의 구조와 C#과 C++ 간의 메모리에 대한 자세한 내용은 @<hd>{basic|basic_unity_output_binary_runtime}을 참고하세요.

//listnum[get_tag][UnityCsReference GameObject.bindings.cs@<fn>{UnityCsReference_GameObject}에서 인용][csharp]{
public extern string tag
{
    [FreeFunction("GameObjectBindings::GetTag", HasExplicitThis = true)]
    get;
    [FreeFunction("GameObjectBindings::SetTag", HasExplicitThis = true)]]
    set;
}
//}

//listnum[get_name][UnityCsReference UnityEngineObject.bindings.cs@<fn>{UnityCsReference_UnityEngineObject}에서 인용][csharp]{
public string name
{
    get { return GetName(this); }
    set { SetName(this, value); }
}

[FreeFunction("UnityEngineObjectBindings::GetName")]
extern static string GetName([NotNull("NullExceptionObject")] Object obj); //}
//}

//footnote[UnityCsReference_GameObject][@<href>{https://github.com/Unity-Technologies/UnityCsReference/blob/c84064be69f20dcf21ebe4 a7bbc176d48e2f289c/Runtime/Export/Scripting/GameObject.bindings.cs}]
//footnote[UnityCsReference_UnityEngineObject][@<href>{https://github.com/Unity-Technologies/UnityCsReference/blob/c84064be69f20 dcf21ebe4a7bbc176d48e2f289c/Runtime/Export/Scripting/UnityEngineObject.bindings.cs}]

=={practice_script_unity_component_cache} 컴포넌트 가져오기
같은 @<code>{GameObject}에 붙어 있는 다른 컴포넌트를 가져오는 @<code>{GetComponent()}도 주의해야 할 것 중 하나입니다.

앞 절의 @<code>{tag} 속성이나 @<code>{name} 속성과 마찬가지로 네이티브 코드로 구현된 처리를 호출하고 있는 것도 마찬가지입니다,
지정한 타입의 컴포넌트를 '검색'하는 비용이 발생한다는 점도 주의해야 합니다.

아래 샘플 코드에서는 매 프레임마다 @<code>{Rigidbody} 컴포넌트를 검색하는 비용이 발생하게 됩니다.
자주 접속하는 경우, 미리 캐싱한 것을 사용하도록 하자.

//listnum[get_component][매 프레임 GetComponent()하는 코드][csharp]{
void Update()
{
    Rigidbody rb = GetComponent<Rigidbody>();
    rb.AddForce(Vector3.up * 10f);
}
//}

=={practice_script_unity_transform} transform 접근하기
<code>{Transform} 컴포넌트는 위치, 회전, 스케일(확대/축소), 부모/자녀 관계 변경 등 자주 접근하는 컴포넌트입니다.
아래 샘플 코드와 같이 여러 값을 갱신하는 경우도 많을 것입니다.

//listnum[sample_transform_NG][transform에 접근하는 예시][csharp]{
void SetTransform(Vector3 position, Quaternion rotation, Vector3 scale)
{
    transform.position = position;
    transform.rotation = rotation;
    transform.localScale = scale;
}
//}

<code>{transform}을 가져오면 Unity 내부에서는 @<code>{GetTransform()}라는 함수가 호출됩니다.
앞 절의 @<code>{GetComponent()}에 비해 최적화되어 있어 빠릅니다.
하지만 캐싱한 경우보다 느리기 때문에 이 역시 아래 샘플 코드처럼 캐싱하여 접근하도록 합니다.
위치와 회전 두 가지는 @<code>{SetPositionAndRotation()}을 사용하여 함수 호출 횟수를 줄일 수도 있습니다.

//listnum[sample_transform_OK][transform을 캐싱하는 예제][csharp]{
void SetTransform(Vector3 position, Quaternion rotation, Vector3 scale)
{
    var transformCache = transform;
    transformCache.SetPositionAndRotation(position, rotation);
    transformCache.localScale = scale;
}
//}

=={practice_script_unity_destroy} 명시적 파기가 필요한 클래스
Unity는 C#으로 개발하기 때문에 GC에 의해 더 이상 참조되지 않는 객체는 해제된다.
하지만 Unity의 일부 클래스는 명시적으로 파기해야 합니다.
대표적인 예로는 @<code>{Texture2D}, @<code>{Sprite}, @<code>{Material}, @<code>{PlayableGraph} 등이 있다.
@<code>{new}나 전용 @<code>{Create} 함수로 생성한 경우 반드시 명시적으로 파기해야 합니다.

//listnum[sample_create][생성 및 명시적 파기][csharp]{
void Start()
{
    _texture = new Texture2D(8, 8);
    _sprite = Sprite.Create(_texture, new Rect(0, 0, 8, 8, 8), Vector2.zero);
    _material = new Material(shader);
    _graph = PlayableGraph.Create();
}

void OnDestroy()
{
    Destroy(_texture);
    Destroy(_sprite);
    Destroy(_material);

    if (_graph.IsValid())
    {
        _graph.Destroy();
    }
}
//}

=={practice_script_unity_keyword_access} 문자열 지정
<code>{Animator}에서 재생할 상태 지정, <code>{Material}에서 조작할 프로퍼티를 지정할 때 문자열을 사용하지 않도록 합니다.

//embed[latex]{
\clearpage
//}

//listnum[sample_keyword_string][문자열 지정 예시][csharp]{
_animator.Play("Wait");
_material.SetFloat("_Prop", 100f);
//}

이들 함수 내부에서는 @<code>{Animator.StringToHash()}와 @<code>{Shader.PropertyToID()}를 실행하여 문자열에서 고유한 식별값으로 변환을 하고 있습니다.
여러 번 접근하는 경우 매번 변환을 하는 것은 낭비이므로, 식별값을 캐쉬에 저장해 두었다가 사용하도록 합니다.
아래 샘플과 같이 캐싱한 식별값의 목록이 되는 클래스를 정의해두면 편리합니다.

//listnum[sample_keyword_cache][식별값 캐시 예시][csharp]{
public static class ShaderProperty
{
    public static readonly int Color = Shader.PropertyToID("_Color");
    public static readonly int Alpha = Shader.PropertyToID("_Alpha");
    public static readonly int ZWrite = Shader.PropertyToID("_ZWrite");
}
public static class AnimationState
{
    public static readonly int Idle = Animator.StringToHash("idle"); }
    public static readonly int Walk = Animator.StringToHash("walk");
    public static readonly int Run = Animator.StringToHash("run");
}
//}

=={practice_script_unity_json_utility} JsonUtility의 함정
Unity에서는 JSON의 직렬화/비직렬화를 위해 @<code>{JsonUtility}라는 클래스가 제공되고 있습니다.
공식 문서 @<fn>{Unity_JSONSerialization}에도 C# 표준보다 빠르다고 명시되어 있어, 성능을 고려한 구현을 한다면 이 클래스를 사용하는 경우가 많을 것입니다.
//footnote[Unity_JSONSerialization][@<href>{https://docs.unity3d.com/ja/current/Manual/JSONSerialization.html}]

//quote{
JsonUtility는 (.NET JSON보다 기능은 적지만) 자주 사용되는 .NET JSON보다 현저하게 빠른 것으로 벤치마크 테스트에서 나타났습니다.
//}

하지만 성능과 관련하여 한 가지 주의해야 할 점이 있습니다.
바로 '@<code>{null} 처리'입니다.

아래 샘플 코드에서 시리얼라이즈 처리와 그 결과를 보여주고 있습니다.
클래스 A의 멤버 b1을 명시적으로 @<code>{null}로 설정했음에도 불구하고, 클래스 B와 클래스 C를 기본 생성자로 생성한 상태로 시리얼라이즈되는 것을 확인할 수 있습니다.
이처럼 직렬화 대상 필드에 @<code>{null}이 있는 경우 JSON화할 때 더미 객체가 @<code>{new}로 생성되므로 그 오버헤드를 고려하는 것이 좋다.

//listnum[sample_json][직렬화 동작][csharp]{
[Serializable] public class A { public B b1; }
[Serializable] public class B { public C c1; public C c2; }
[Serializable] public class C { public int n; }

void Start()
{
    Debug.Log(JsonUtility.ToJson(new A() { b1 = null, })); }
    // {"b1":{"c1":{"n":0},"c2":{"n":0}}}
}
//}

=={practice_script_unity_material_leak} Render 및 MeshFilter의 함정
@<code>{Renderer.material}에서 가져온 머티리얼, @<code>{MeshFilter.mesh}에서 가져온 메시는 복제된 인스턴스이기 때문에
사용 후에는 명시적으로 파기해야 합니다.
공식 문서 @<fn>{Unity_Renderer_material}@<fn>{Unity_MeshFilter_mesh}에도 각각 아래와 같이 명시되어 있습니다.
//footnote[Unity_Renderer_material][@<href>{https://docs.unity3d.com/ja/current/ScriptReference/Renderer-material.html}]
//footnote[Unity_MeshFilter_mesh][@<href>{https://docs.unity3d.com/ja/current/ScriptReference/MeshFilter-mesh.html}]

//quote{
이 머티리얼을 다른 렌더러에서 사용하는 경우, 공유된 머티리얼을 복제하여 지금부터 사용하게 됩니다.
//}

//quote{
게임 오브젝트를 파괴할 때 자동으로 인스턴스화된 메시를 파괴하는 것은 사용자의 책임입니다.
//}

획득한 머티리얼과 메시는 멤버 변수에 보관해 두었다가 적절한 시점에 파기하도록 하자.

//listnum[sample_destroy_copy_material][복제된 머티리얼의 명시적 파기][csharp]{
void Start()
{
    _material = GetComponent<Renderer>().material;
}

void OnDestroy()
{
    if (_material ! = null) {
        Destroy(_material);
    }
}
//}


=={practice_script_unity_log_delete} 로그 출력 코드 제거
Unity에서는 @<code>{Debug.Log()}, @<code>{Debug.LogWarning()}, @<code>{Debug.LogError()}와 같은 로그 출력용 함수가 제공된다.
편리한 기능이지만 몇 가지 문제점이 있습니다.

 * 로그 출력 자체의 처리량이 다소 무겁다.
 * 릴리즈 빌드에서도 실행된다.
 * 문자열 생성 및 연결로 인해 GC.Alloc이 발생한다.

Unity의 Logging 설정을 끄면 스택 트레이싱은 중지되지만, 로그는 출력됩니다.
Unity에서 @<code>{UnityEngine.Debug.unityLogger.logEnabled}에 @<code>{false}를 설정하면 로그가 출력되지 않습니다,
함수 내부에서 분기만 하기 때문에 함수 호출 비용과 불필요한 문자열 생성 및 연결은 이루어집니다.
@<code>{#if} 지시어를 사용하는 방법도 있지만, 모든 로그 출력 처리를 일일이 신경 쓰는 것은 현실적이지 않다.

//listnum[sample_directive][#if 지시어][csharp]{
#if UNITY_EDITOR
  Debug.LogError($"Error {e}"); #endif
#endif
//}

이런 경우에 활용할 수 있는 것이 @<code>{Conditional} 속성이다.
@<code>{Conditional} 속성이 붙은 함수는 지정한 심볼이 정의되어 있지 않으면 컴파일러가 호출 부분을 제거합니다.
@<list>{sample_conditional}의 샘플과 같이 자체 제작한 로그 출력 클래스를 통해 Unity 측의 로그 기능을 호출하는 것을 원칙으로 하고, 자체 제작 클래스 측의 각 함수에 @<code>{Conditional} 속성을 추가하면 됩니다,
필요에 따라 함수 호출마다 제거할 수 있도록 하는 것이 좋습니다.

//embed[latex]{
\clearpage
//}

//listnum[sample_conditional][Conditional 속성 예시][csharp]{
public static class Debug
{
    private const string MConditionalDefine = "DEBUG_LOG_ON";

    [System.Diagnostics.Conditional(MConditionalDefine)]
    public static void Log(object message)
        => UnityEngine.Debug.Log(message); }
}
//}

주의할 점은 지정한 심볼이 함수 호출 측에서 참조할 수 있어야 한다는 점입니다.
@<code>{#define}에서 정의한 심볼의 범위는 작성한 파일 내로 한정됩니다.
@<code>{Conditional} 속성이 붙은 함수를 호출하는 모든 파일에 심볼을 정의하는 것은 현실적으로 불가능합니다.
Unity에는 Scripting Define Symbols라는 프로젝트 전체에 대해 심볼을 정의할 수 있는 기능이 있으니 활용해보자.
'Project Settings -> Player -> Other Settings'에서 설정할 수 있습니다.

//image[practice_script_unity_define][Scripting Define Symbols] =={practice_script_unity_define

=={practice_script_unity_burst} Burst를 이용한 코드 속도 향상

Burst@<fn>{burst}는 유니티 공식에서 개발한 고성능 C# 스크립팅을 위한 컴파일러입니다.
//footnote[burst][@<href>{https://docs.unity3d.com/Packages/com.unity.burst@1.6/manual/docs/QuickStart.html}]

Burst에서는 C#의 하위 언어를 사용하여 코드를 작성합니다.
Burst가 C# 코드를 LLVM이라는 컴파일러 기반 @<fn>{llvm}의 중간 구문인 IR(Intermediate Representation)로 변환하고, IR을 최적화한 후 기계어로 변환합니다.

//footnote[llvm][@<href>{https://llvm.org/}]

이때 코드를 최대한 벡터화하여 SIMD라는 명령어를 적극적으로 사용하는 처리로 대체합니다. 이를 통해 보다 빠른 프로그램 출력을 기대할 수 있습니다.

SIMD는 Single Instruction/Multiple Data의 약자로, 하나의 명령어를 동시에 여러 데이터에 적용하는 명령어를 말합니다.
즉, SIMD 명령어를 적극적으로 활용하면 하나의 명령어로 데이터를 한꺼번에 처리하기 때문에 일반 명령어에 비해 빠르게 동작한다.

==={practice_script_csharp_optimize_code_with_burst} Burst를 이용한 코드의 고속화

Burst에서는 High Performance C#(HPC#)@<fn>{burst_hpc}라는 C#의 하위 언어를 사용하여 코드를 작성합니다.
//footnote[burst_hpc][@<href>{https://docs.unity3d.com/Packages/com.unity.burst@1.7/manual/docs/CSharpLanguageSupport_Types.html}]

HPC#의 특징 중 하나로 C#의 참조형, 즉 클래스나 배열 등을 사용할 수 없습니다. 따라서 원칙적으로 구조체를 사용하여 데이터 구조를 기술합니다.

배열과 같은 컬렉션은 대신 @<code>{NativeArray<T>} 등의 NativeContainer@<fn>{burst_native_container}를 이용하며, HPC#에 대한 자세한 내용은 각주에 기재된 문서를 참고하시기 바랍니다.
//footnote[burst_native_container][@<href>{https://docs.unity3d.com/Manual/JobSystemNativeContainer.html}]

Burst는 C# Job System과 함께 사용합니다. 따라서 자신의 처리를 @<code>{IJob}을 구현한 Job의 @<code>{Execute} 메소드 내에 작성합니다.
정의한 Job에 @<code>{BurstCompile} 속성을 부여하면 해당 Job이 Burst에 의해 최적화됩니다.

다음은 @<list>{burst_job}에 주어진 배열의 각 요소를 제곱하여 @<code>{Output} 배열에 저장하는 예제입니다.

//listnum[burst_job][간단한 검증을 위한 Job 구현][csharp]{
[BurstCompile].
private struct MyJob : IJob
MyJob : IJob
    [ReadOnly][csharp
    public NativeArray<float> Input;

    [WriteOnly]
    public NativeArray<float> Output; [WriteOnly] [ReadOnly

    public void Execute()
    {
        for (int i = 0; i < Input.Length; i++)
        {
            Output[i] = Input[i] * Input[i];
        }
    }
}
//}

<list>{burst_job}의 14번째 줄의 각 요소는 각각 독립적으로 계산할 수 있고(계산에 순서 의존성이 없음), 출력 배열의 메모리 정렬이 연속적이기 때문에 SIMD 명령어를 사용하여 일괄적으로 계산할 수 있다.

코드가 어떤 어셈블리로 변환되는지는 @<img>{burst_inspector}와 같이 Burst Inspector를 통해 확인할 수 있다.

//image[burst_inspector][Burst Inspector를 이용하면 코드가 어떤 어셈블리로 변환되는지 확인할 수 있다][scale=1.0][scale=1.0].

<list>{burst_job}의 14번째 줄의 처리는 ARMV8A_AARCH64용 어셈블리에서 @<list>{burst_job_asm_simd}로 변환된다.

//listnum[burst_job_asm_simd][@<list>{burst_job}의 14번째 줄의 ARMV8A_AARCH64용 어셈블리]{
        fmul v0.4s, v0.4s, v0.4s
        fmul v1.4s, v1.4s, v1.4s
//}

어셈블리의 피연산자에 @<code>{.4s}라는 접미사가 붙은 것을 보면 SIMD 명령어를 사용하고 있음을 확인할 수 있다.

순수 C#으로 구현한 코드와 Burst로 최적화한 코드의 성능을 실제 기기에서 비교한다.

실기에는 Android Pixel 4a, IL2CPP를 스크립트 백엔드로 빌드하여 비교하였습니다. 또한 배열의 크기는 2^20 = 1,048,576으로 설정했다.
측정은 동일한 처리를 10회 반복하여 처리시간의 평균을 취했다.

<table>{burst_comp}에 성능 비교 측정 결과가 나와 있습니다.

//tsize[|latex|||l|r|]
//table[burst_comp][순수 C# 구현과 Burst로 최적화한 코드의 처리 시간 비교]{
방법론 처리시간(숨김)
-------------------------------------------------------------
순수 C# 구현 5.73ms
Burst 구현 0.98ms
//}

순수 C# 구현에 비해 약 5.8배 정도 빠른 속도를 확인할 수 있었습니다.