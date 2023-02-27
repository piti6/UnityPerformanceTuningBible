={practice_ui} 튜닝 실습 - UI
Unity 표준 UI 시스템인 uGUI와 화면에 텍스트를 그리는 메커니즘인 TextMeshPro에 대해 알아봅니다,
튜닝 실습을 소개합니다.

=={practice_ui_rebuild} Canvas의 분할

uGUI에서는 @<code>{Canvas} 내의 요소에 변화가 있을 때,
전체 UI의 메시를 재구성하는 처리(리빌드)가 실행됩니다.
변화란 액티브 전환이나 이동, 크기 변경 등입니다,
외형이 크게 바뀌는 것부터 겉으로 보기에는 보이지 않는 미세한 것까지 모든 변화를 의미합니다.
리빌드 처리에 드는 비용이 높기 때문에 실행 횟수가 많거나
@<code>{Canvas} 내 UI의 개수가 많으면 성능에 악영향을 미친다.

이에 반해 @<code>{Canvas}를 어느 정도의 UI 덩어리마다 분할하면,
리빌드 비용을 줄일 수 있습니다.
예를 들어, 애니메이션으로 움직이는 UI와 아무것도 움직이지 않는 UI가 있다고 가정해보자,
그것들을 별도의 @<code>{Canvas} 아래에 배치하면 된다,
애니메이션에 의한 리빌드 대상을 최소화할 수 있습니다.

단, @<code>{Canvas}를 분할하면 드로잉 배치가 작동하지 않으므로 주의해야 합니다,
분할하는 방법에 대해서는 신중하게 고려해야 합니다.

//info{

Canvas}의 분할은 @<code>{Canvas} 아래에 @<code>{Canvas}를 중첩하여 배치하는 경우에도 유효합니다.
자식 @<code>{Canvas}에 포함된 요소가 변경되어도 자식 @<code>{Canvas}의 리빌드만 실행되고 부모 @<code>{Canvas}의 리빌드는 실행되지 않습니다.
하지만 자세히 확인해 보니 @<code>{SetActive}에 의해 자식 @<code>{Canvas} 내의 UI를 활성 상태로 전환했을 때는 사정이 다른 것 같습니다.
이때 부모 @<code>{Canvas} 내에 UI가 많이 배치되어 있는 경우 고부하가 걸리는 현상이 있는 것 같습니다.
왜 그런 현상이 발생하는지 자세한 이유는 알 수 없지만, 중첩된 @<code>{Canvas} 내 UI의 활성 상태를 전환할 때는 주의가 필요해 보입니다.

//}

=={practice_ui_unity_white} UnityWhite

UI를 개발하다 보면 단순한 직사각형 형태의 오브젝트를 표시하고 싶을 때가 종종 있습니다.
이때 주목해야 할 것이 바로 UnityWhite의 존재입니다.
UnityWhite는 @<code>{Image} 컴포넌트나 @<code>{RawImage} 컴포넌트에서
사용할 이미지를 지정하지 않았을 때(@<img>{none_image}) 사용되는 Unity 내장 텍스처입니다.
UnityWhite가 사용되는 모습은 Frame Debugger에서 확인할 수 있습니다(@<img>{unity_white}).
이 구조를 사용하면 흰색 직사각형을 그릴 수 있기 때문입니다,
여기에 곱하는 색을 조합하여 간단한 직사각형 형태의 디스플레이를 구현할 수 있습니다.

//image[none_image][UnityWhite 활용][scale=0.8][scale=0.8].
//image[unity_white][UnityWhite 사용 모습][scale=0.8][scale=0.8].

하지만 UnityWhite는 프로젝트에서 준비한 SpriteAtlas와는 다른 텍스처이기 때문에,
드로잉 배치가 끊기는 문제가 발생합니다.
이로 인해 드로우 호출이 증가하여 드로잉 효율이 떨어지게 됩니다.

따라서 SpriteAtlas에 작은(예를 들어 4×4 픽셀의) 흰색 사각형 이미지를 추가합니다,
그 Sprite를 이용하여 단순한 직사각형을 그리도록 해야 합니다.
이렇게 하면 동일한 SpriteAtlas를 사용하면 동일한 머티리얼이 되므로 일괄 처리할 수 있습니다.

=={practice_ui_layout_component} Layout 컴포넌트

uGUI에는 오브젝트를 깔끔하게 정렬하는 기능을 가진 Layout 컴포넌트가 있습니다.
예를 들어 세로로 정렬하려면 @<code>{VerticalLayoutGroup},
그리드 위에 정렬하려면 @<code>{GridLayoutGroup}이 사용됩니다(@<img>{layout_group}).

//image[layout_group][왼쪽이 @<code>{VerticalLayoutGroup}, 오른쪽이 @<code>{GridLayoutGroup}을 사용한 예시][scale=0.99][scale=0.99].

Layout 컴포넌트를 이용하면 대상 객체를 생성하거나 특정 속성을 편집할 때 Layout의 리빌드가 발생한다.
Layout 재구축도 메시 재구축과 마찬가지로 비용이 많이 드는 작업입니다.

Layout 재구축으로 인한 성능 저하를 방지하려면,
Layout 컴포넌트를 최대한 사용하지 않는 것이 효과적입니다.

예를 들어, 텍스트의 내용에 따라 배치가 바뀌는 등 동적 배치가 필요하지 않다면,
Layout 컴포넌트를 사용할 필요가 없습니다.
정말 동적인 배치가 필요한 경우도, 화면에서 많이 사용되는 경우 등은,
자체 스크립트로 제어하는 것이 좋을 수도 있습니다.
또한, 부모의 크기가 변해도 부모로부터 볼 때 특정 위치에 배치하고 싶은 요구사항이라면,
@<code>{RectTransform}의 앵커를 조정하면 됩니다.
만약 프리팹을 만들 때 배치에 편리하다는 이유로 Layout 컴포넌트를 사용했다면,
반드시 삭제하고 저장하도록 합니다.

=={practice_ui_raycast_target} 레이캐스트 타깃(Raycast Target)
Image}나 @<code>{Image}나 @<code>{RawImage}의 베이스 클래스인 @<code>{Graphic}에는,
Raycast Target이라는 속성이 있습니다(@<img>{raycast_target}).
이 속성을 활성화하면 해당 @<code>{Graphic}이 클릭이나 터치의 대상이 됩니다.
화면을 클릭하거나 터치할 때 이 속성이 활성화된 오브젝트가
처리 대상이 되므로 가능한 한 이 속성을 비활성화하여 성능을 향상시킬 수 있습니다.

//image[raycast_target][Raycast Target 속성][scale=0.8].

이 속성은 기본적으로 활성화되어 있습니다,
실제로 많은 @<code>{Graphic}에서는 이 속성을 활성화할 필요가 없습니다.
한편, 유니티에는 프리셋 @<fn>{preset}이라는 기능이 있어 프로젝트에서 기본값을 변경할 수 있습니다.
구체적으로 @<code>{Image} 컴포넌트와 @<code>{RawImage} 컴포넌트에 대해 각각 프리셋을 생성합니다,
이를 Project Settings의 프리셋 관리자에서 기본 프리셋으로 등록합니다.
이 기능을 사용하여 Raycast Target 프로퍼티를 기본적으로 비활성화할 수도 있습니다.

//footnote[preset][@<href>{https://docs.unity3d.com/ja/current/Manual/Presets.html}]

=={practice_ui_mask} 마스크

uGUI에서 마스크를 표현하려면 @<code>{Mask} 컴포넌트 또는 @<code>{RectMask2d} 컴포넌트를 사용합니다.

Mask}는 스텐실을 이용하여 마스크를 구현하기 때문에 컴포넌트가 늘어날 때마다 그리기 비용이 커집니다.
반면 @<code>{RectMask2d}는 셰이더의 파라미터로 마스크를 구현하기 때문에 그리기 비용의 증가를 억제할 수 있습니다.
단, @<code>{Mask}는 원하는 형태로 속을 비울 수 있는 반면, @<code>{RectMask2d}는 직사각형으로만 속을 비울 수 있다는 제약이 있습니다.

사용할 수 있다면 @<code>{RectMask2d}를 선택해야 한다는 것이 통설입니다,
최근 Unity에서는 @<code>{RectMask2d}의 사용에도 주의가 필요합니다.

구체적으로는 @<code>{RectMask2d}가 유효할 때, 그 마스크 대상이 늘어날수록,
그에 비례하여 매 프레임 컬링의 CPU 부하가 발생합니다.
UI를 전혀 움직이지 않아도 매 프레임마다 부하가 발생하는 이 현상은 uGUI 내부 구현의 코멘트를 보면 Unity 2019.3에서 들어간 어떤 issue@<fn>{mask_issue} 수정의
부작용으로 인한 것으로 보입니다.
//footnote[mask_issue][@<href>{https://issuetracker.unity3d.com/issues/rectmask2d-diffrently-masks-image-in-the-play-mode-when- animating-rect-transform-pivot-property}]]

따라서 @<code>{RectMask2d}도 가급적 사용하지 않도록 한다,
사용하더라도 필요 없는 상태일 때는 @<code>{enabled}를 @<code>{false}로 설정한다,
마스크 대상은 필요 최소한으로 하는 등의 조치를 취하는 것이 효과적입니다.

=={practice_ui_text_mesh_pro} TextMeshPro
TextMeshPro에서 텍스트를 설정하는 일반적인 방법은 @<code>{text} 속성에 텍스트를 대입하는 방법입니다,
이와는 별도로 @<code>{SetText}라는 메소드를 사용하는 방법이 있습니다.

SetText}에는 많은 오버로드가 존재하는데요,
예를 들어 문자열과 @<code>{float} 타입의 값을 인수로 받는 것이 있습니다.
이 메서드를 @<list>{setText}와 같이 사용하면 두 번째 인수의 값을 표시할 수 있습니다.
단, @<code>{label}은 @<code>{TMP_Text}(또는 이를 상속받은) 타입,
@<code>{number}는 @<code>{float} 타입의 변수라고 가정한다.

//listnum[setText][SetText 활용 예시][csharp]{
label.SetText("{0}", number);
//}

이 방법의 장점은 문자열 생성 비용을 줄일 수 있다는 점입니다.

//listnum[noSetText][SetText를 사용하지 않는 예][csharp]{
label.text = number.ToString(); //}
//}

@<list>{noSetText}와 같이 @<code>{text} 속성을 사용하는 방법으로는,
@<code>{float} 타입의 @<code>{ToString()}이 실행되기 때문에 이 처리가 실행될 때마다 문자열 생성 비용이 발생한다.
이에 반해 @<code>{SetText}를 사용한 방법은 문자열을 최대한 생성하지 않도록 고안되었기 때문이다,
특히 자주 표시하는 텍스트가 바뀌는 경우 성능적으로 유리합니다.

또한 이 TextMeshPro의 기능은 ZString@<fn>{zstring}과 함께 사용하면 매우 강력해진다.
ZString은 문자열 생성 시 메모리 할당을 줄일 수 있는 라이브러리입니다.
ZString은 @<code>{TMP_Text} 타입에 대한 많은 확장 메소드를 제공합니다,
이 메서드들을 사용하면 문자열 생성 비용을 줄이면서 유연한 텍스트 표시를 구현할 수 있습니다.

//footnote[zstring][@<href>{https://github.com/Cysharp/ZString}]

=={practice_ui_active} UI의 표시 전환

uGUI의 컴포넌트는 @<code>{SetActive}에 의한 객체 활성 전환 비용이 크다는 특징이 있다.
이는 @<code>{OnEnable}에서 각종 리빌드 Dirty 플래그를 설정하거나 마스크 관련 초기화를 하기 때문입니다.
따라서 UI의 표시/비표시 전환 방법으로 @<code>{SetActive}에 의한 방법 외의 대안도 고려하는 것이 중요합니다.

첫 번째 방법은 @<code>{Canvas}의 @<code>{enabled}를 @<code>{false}로 설정하는 방법입니다(@<img>{canvas_disable}).
이렇게 하면 @<code>{Canvas} 아래 있는 모든 오브젝트가 그려지지 않게 됩니다.
따라서 이 방법은 @<code>{Canvas} 아래의 오브젝트를 통째로 숨기고 싶은 경우에만 사용할 수 있다는 단점이 있습니다.

//image[canvas_disable][@<code>{Canvas} 비활성화][scale=0.8][scale=0.8].

또 다른 방법은 @<code>{CanvasGroup}을 사용하는 방법이다.
<code>{CanvasGroup}에는 그 아래에 있는 오브젝트의 투명도를 일괄적으로 조정할 수 있는 기능이 있습니다.
이 기능을 이용하여 투명도를 0으로 설정하면 됩니다,
해당 @<code>{CanvasGroup} 아래 모든 오브젝트를 숨길 수 있습니다(@<img>{canvas_group}).

//image[canvas_group][@<code>{CanvasGroup}의 투명도 0으로 설정][scale=0.8][scale=0.8].

이 방법들은 @<code>{SetActive}로 인한 부하를 피할 수 있을 것으로 기대할 수 있습니다,
GameObject}는 활성 상태로 유지되므로 주의가 필요한 경우도 있습니다.
예를 들어 @<code>{Update} 메소드가 정의되어 있는 경우, 숨겨져 있는 상태에서도 해당 처리가 계속 실행되기 때문입니다,
예상치 못한 부하 증가로 이어질 수 있으므로 주의해야 합니다.

참고로 @<code>{Image} 컴포넌트를 붙인 1280개의 @<code>{GameObject}에 대해,
각각의 방법으로 표시/비표시 전환을 했을 때의 처리 시간을 측정했습니다(@<table>{activation_time}).
처리 시간은 Unity 에디터에서 측정했으며, Deep Profile은 사용하지 않았습니다.
실제로 전환을 한 바로 그 처리의 실행 시간 @<fn>{activation_profile}과,
해당 프레임에서 @<code>{UIEvents.WillRenderCanvases}의 실행 시간을 합산한 것을
해당 기법의 처리 시간으로 사용합니다.
<code>{UIEvents.WillRenderCanvases}의 실행 시간을 합산한 것입니다,
이 안에서 UI 재구축이 이루어지기 때문입니다.

//footnote[activation_profile][예를 들어 @<code>{SetActive}라면 @<code>{SetActive} 메소드를 호출하는 부분을 @<code>{Profiler.BeginSample}와 @<code>{ Profiler.EndSample}로 둘러싸서 측정하고 있습니다].

#@# SetActive 210.41+113.38=323.79 208.24+1.69=209.93
#@# Canva 8.99+52.26=61.25 9.07+52.16=61.23
#@# CanvasGroup 0.95+2.69=3.64 0.97+2.43=3.40

//tsize[|latex||l|r|r|]
//table[activation_time][표시전환 처리시간]{
방법 처리 시간(표시) 처리 시간(숨김)
-------------------------------------------------------------
@<code>{SetActive} 323.79ms 209.93ms
@<code>{Canvas}의 @<code>{enabled} 61.25ms 61.23ms 61.23ms
@<code>{CanvasGroup}의 @<code>{alpha} 3.64ms 3.40ms 3.40ms
//}

@<table>{activation_time}의 결과에서,
이번 상황에서 CanvasGroup을 사용한 방식이 압도적으로 처리 시간이 짧다는 것을 알 수 있다.
