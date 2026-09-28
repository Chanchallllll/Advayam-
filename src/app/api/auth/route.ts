export async function GET() {
  return Response.json({
    message: "Advayam auth API is working",
  });
}

export async function POST(request: Request) {
  const body = await request.json();

  return Response.json({
    message: "Auth request received",
    data: body,
  });
}
