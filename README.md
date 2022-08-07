# Md2-Loader
A Md2 loader written mostly in c but uses a namespace and one class, so should be easily portable to pure C. 
Also the loader generates vertex normals and supports animations.

![Software FPS_ 244 ms_ 4 098 07 08 2022 01_56_58](https://user-images.githubusercontent.com/55063400/183269477-12fa5d32-f23d-4b60-b721-6e67320bbfae.png)
![GL Software 07 08 2022 01_57_19](https://user-images.githubusercontent.com/55063400/183269485-67b8a600-4595-4dcf-be0c-c3aa6ec4995d.png)

```C
Md2::ModelData Model;
Md2::Load("Solider.md2", Texture.Width, Texture.Height, &Model);

Md2::Proccess ProcModel;
ProcModel.Model = &Model;
ProcModel.Start(AnimationStart, AnimationEnd);

for (int i = 0; i < Model.NumTriangles; i++)
{
  ProcModel.Update(i);

  Triangle.v0.Pos.x = ProcModel.Vertex[0].P[0];
  Triangle.v0.Pos.y = ProcModel.Vertex[0].P[1];
  Triangle.v0.Pos.z = ProcModel.Vertex[0].P[2];
  Triangle.v0.Pos.w = 1.0;
  Triangle.v0.TexCoord.x = ProcModel.TexCoord[0].S;
  Triangle.v0.TexCoord.y = ProcModel.TexCoord[0].T;
  Triangle.v0.Normal.x = ProcModel.Normal[0].P[0];
  Triangle.v0.Normal.y = ProcModel.Normal[0].P[1];
  Triangle.v0.Normal.z = ProcModel.Normal[0].P[2];

  Triangle.v1.Pos.x = ProcModel.Vertex[2].P[0];
  Triangle.v1.Pos.y = ProcModel.Vertex[2].P[1];
  Triangle.v1.Pos.z = ProcModel.Vertex[2].P[2];
  Triangle.v1.Pos.w = 1.0;
  Triangle.v1.TexCoord.x = ProcModel.TexCoord[2].S;
  Triangle.v1.TexCoord.y = ProcModel.TexCoord[2].T;
  Triangle.v1.Normal.x = ProcModel.Normal[2].P[0];
  Triangle.v1.Normal.y = ProcModel.Normal[2].P[1];
  Triangle.v1.Normal.z = ProcModel.Normal[2].P[2];

  Triangle.v2.Pos.x = ProcModel.Vertex[1].P[0];
  Triangle.v2.Pos.y = ProcModel.Vertex[1].P[1];
  Triangle.v2.Pos.z = ProcModel.Vertex[1].P[2];
  Triangle.v2.Pos.w = 1.0;
  Triangle.v2.TexCoord.x = ProcModel.TexCoord[1].S;
  Triangle.v2.TexCoord.y = ProcModel.TexCoord[1].T;
  Triangle.v2.Normal.x = ProcModel.Normal[1].P[0];
  Triangle.v2.Normal.y = ProcModel.Normal[1].P[1];
  Triangle.v2.Normal.z = ProcModel.Normal[1].P[2];

  glDrawTriangle(&Triangle);
}

ProcModel.End(DeltaTime * 25.0);
```
